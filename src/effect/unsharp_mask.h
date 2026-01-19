#pragma once

_PHOXO_BEGIN
_PHOXO_EFFECT_BEGIN

/// Unsharp Mask for image sharpening (32 bit).
class UnsharpMask : public PixelIterator<UnsharpMask>
{
private:
    Image   m_bak;
    int   m_amount;
    int   m_threshold;
    StackBlur   m_gauss_blur;

public:
    /// radius >= 1 \n 1 <= amount <= 100 \n 0 <= threshold <= 255.
    UnsharpMask(int radius, int amount, int threshold) : m_gauss_blur(radius)
    {
        m_amount = std::clamp(amount, 1, 100);
        m_threshold = std::clamp(threshold, 0, 255);
    }

    static void HandlePixel(Image& img, int x, int y, Color* px, UnsharpMask& eff)
    {
        eff.Process(x, y, *px);
    }

private:
    void OnBeforeProcess(Image& img) override
    {
        m_bak = img;

        m_gauss_blur.EnableParallel(IsParallelEnabled());
        img.ApplyEffect(m_gauss_blur);
    }

    void ProcessChannel(BYTE& dst, BYTE origin) const
    {
        int   diff = origin - dst;
        if (abs(diff) < m_threshold)
            dst = origin;
        else
            dst = Math::Clamp0255i(origin + m_amount * diff / 100);
    }

    void Process(int x, int y, Color& px) const
    {
        auto   origin = (Color*)m_bak.GetPixel(x, y);
        ProcessChannel(px.r, origin->r);
        ProcessChannel(px.g, origin->g);
        ProcessChannel(px.b, origin->b);
        px.a = origin->a;
    }
};

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
