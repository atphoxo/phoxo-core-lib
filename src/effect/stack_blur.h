#pragma once
#include <numeric>

_PHOXO_BEGIN
_PHOXO_EFFECT_BEGIN

// The branch introduced by 'bool m_only_blur_alpha' will significantly slow down performance
struct BlurParams
{
    int   r;
    bool   m_copy_edge;
};

/// @cond
_PHOXO_INTERNAL_BEGIN
struct KernelInfo : public std::vector<int>,
                    public BlurParams
{
    KernelInfo(const auto& param) : BlurParams(param)
    {
        r = std::clamp(r, 1, 500);
        resize(2 * r + 1);
        std::iota(begin(), begin() + r + 1, 1); // generate: 1 2 3 4 5 4 3 2 1
        std::iota(rbegin(), rbegin() + r, 1);
        stack_count = (r + 1) * (r + 1);
    }

    int   stack_count;
};

using KernelCRef = const KernelInfo&;

struct LineBuffer : public std::vector<Color>
{
    KernelCRef   m_kernel;
    const int   m_width_or_height;

    LineBuffer(const auto& kernel, int width_or_height) : m_kernel(kernel), m_width_or_height(width_or_height)
    {
        resize(width_or_height + 2 * kernel.r + 4); // +2 is enough
    }

    void GetHorizontalLine(const Image& img, int y)
    {
        auto   curr = data();
        FillPadding(curr, img.GetPixel(0, y));
        for (int x = 0; x < img.Width(); x++)
        {
            *curr++ = *(Color*)img.GetPixel(x, y);
        }
        FillPadding(curr, img.GetPixel(img.Width() - 1, y));
    }

    void GetVertLine(const Image& img, int x)
    {
        auto   curr = data();
        FillPadding(curr, img.GetPixel(x, 0));
        for (int y = 0; y < img.Height(); y++)
        {
            *curr++ = *(Color*)img.GetPixel(x, y);
        }
        FillPadding(curr, img.GetPixel(x, img.Height() - 1));
    }

private:
    void FillPadding(Color*& curr, const void* edge_pixel) const
    {
        auto   t = (m_kernel.m_copy_edge ? *(const Color*)edge_pixel : Color{});
        curr = std::fill_n(curr, m_kernel.r, t);
    }
};

class StackSum
{
private:
    KernelCRef   m_kernel;
    double   sr = 0, sg = 0, sb = 0, sa = 0;

public:
    StackSum(const auto& kn) : m_kernel(kn) {}

    void Add(const auto* px) { AddWeight(px, 1); }
    void Sub(const auto* px) { AddWeight(px, -1); }

    void AddWeight(const Color* px, int weight)
    {
        px->PremulSum(sb, sg, sr, sa, weight);
    }

    void operator-=(const StackSum& s) { sb -= s.sb; sg -= s.sg; sr -= s.sr; sa -= s.sa; }
    void operator+=(const StackSum& s) { sb += s.sb; sg += s.sg; sr += s.sr; sa += s.sa; }

    void Output(Color* p) const
    {
        if (sa > 0)
        {
            p->b = (BYTE)(sb / sa + 0.5);
            p->g = (BYTE)(sg / sa + 0.5);
            p->r = (BYTE)(sr / sa + 0.5);
            p->a = (BYTE)(sa / m_kernel.stack_count + 0.5);
        }
        else
        {
            p->val = 0;
        }
    }
};

struct InputPixelView
{
    const Color   * m_begin, * m_split, * m_end; // ->1 2 3 4 ->5 6 7 ->X

    InputPixelView(const auto& buf) : m_begin(buf.data())
    {
        m_split = m_begin + buf.m_kernel.r + 1;
        m_end = m_begin + buf.m_kernel.size() + 1; // 因为计算first时in多+1
    }

    void Step() { m_begin++; m_split++; m_end++; }
};

class LineCalculator
{
private:
    const LineBuffer   & m_buf;
    StackSum   m_out, m_in, m_stack;

public:
    LineCalculator(const auto& buf) : m_buf(buf), m_out(buf.m_kernel), m_in(buf.m_kernel), m_stack(buf.m_kernel) {}

    void OutputLine(Color* dest, int pixel_span)
    {
        SumFirstKernel(m_buf.data());

        InputPixelView   viewer(m_buf);
        for (int i = 0; i < m_buf.m_width_or_height; i++, dest += pixel_span, viewer.Step())
        {
            m_stack.Output(dest);
            m_stack -= m_out;
            m_stack += m_in;

            if (*viewer.m_end != *viewer.m_split)
            {
                m_in.Add(viewer.m_end);
                m_in.Sub(viewer.m_split);
            }

            if (*viewer.m_begin != *viewer.m_split)
            {
                m_out.Add(viewer.m_split);
                m_out.Sub(viewer.m_begin);
            }
        }
    }

private:
    void SumFirstKernel(const Color* px)
    {
        for (int i = 0; int weight : m_buf.m_kernel)
        {
            if (i <= m_buf.m_kernel.r)
                m_out.Add(px);
            else
                m_in.Add(px);
            m_stack.AddWeight(px, weight);
            i++; px++;
        }
        m_in.Add(px); // 可能会越界，越界也没问题，反正是+0
    }
};

class StackBlurAxisBase : public ImageEffect
{
protected:
    KernelCRef   m_kernel;
public:
    StackBlurAxisBase(const auto& kn) : m_kernel(kn) {}
private:
    bool IsSupported(const Image& img) final { return true; }
};

class StackBlurHorz : public StackBlurAxisBase
{
    using StackBlurAxisBase::StackBlurAxisBase;
    SIZE GetScanLineCountPerTask(const Image& img) override { return { 0, 200 }; }

    void ProcessRegion(Image& img, CRect rc, IProgressListener* progress) override
    {
        LineBuffer   lnbuf(m_kernel, img.Width()); // Since this function may be called from other threads, it requires its own scan line buffer.
        for (int y = rc.top; y < rc.bottom; y++)
        {
            lnbuf.GetHorizontalLine(img, y);

            LineCalculator   calc(lnbuf);
            calc.OutputLine((Color*)img.GetLinePtr(y), 1);
        }
    }
};

class StackBlurVert : public StackBlurAxisBase
{
    using StackBlurAxisBase::StackBlurAxisBase;
    SIZE GetScanLineCountPerTask(const Image& img) override { return { 200, 0 }; }

    void ProcessRegion(Image& img, CRect rc, IProgressListener* progress) override
    {
        LineBuffer   lnbuf(m_kernel, img.Height());
        for (int x = rc.left; x < rc.right; x++)
        {
            lnbuf.GetVertLine(img, x);

            LineCalculator   calc(lnbuf);
            calc.OutputLine((Color*)img.GetPixel(x, 0), img.Width());
        }
    }
};
_PHOXO_NAMESPACE_END
/// @endcond

/// Stack blur (32 bit).
class StackBlur : public ImageEffect
{
private:
    const internal::KernelInfo   m_kernel;

public:
    StackBlur(int radius) : StackBlur(BlurParams{ radius, true }) {}
    StackBlur(const BlurParams& param) : m_kernel(param) {}

private:
    ProcessMode QueryProcessMode() override
    {
        return ProcessMode::EntireMyself;
    }

    void ProcessEntire(Image& img, IProgressListener* progress) override
    {
        if (progress) { progress->BeginFixProgress(0); }

        internal::StackBlurVert   vert(m_kernel); // 8bit改造这两个effect可以替换成模板
        internal::StackBlurHorz   horz(m_kernel);

        ImageEffect*   effects[] = { &vert, &horz };
        for (auto& eff : effects)
        {
            eff->EnableParallel(IsParallelEnabled());
            img.ApplyEffect(*eff, progress);
        }

        if (progress) { progress->EndFixProgress(); }
    }
};

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
