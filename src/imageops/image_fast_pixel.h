#pragma once

_PHOXO_BEGIN

/// Fast per-pixel utilities for 32-bit images
class ImageFastPixel
{
public:
    /// parallel if pixels exceed
    static inline int   parallel_threshold = 500'000;

    /// Returns true if all pixels have alpha == 0xFF
    static bool IsFullyOpaque(const Image& img)
    {
        PixelSpan   pv{ img };
        if (!pv)
            return false;

        auto   func = [](const auto& px) { return px.a == 0xFF; };

        if (pv.use_parallel)
            return std::all_of(std::execution::par, pv.begin, pv.end, func);
        else
            return std::all_of(pv.begin, pv.end, func);
    }

    /// Fills RGB of all pixels to the specified color, keeping alpha unchanged
    static void FillRGBKeepAlpha(Image& img, Color clr)
    {
        if (PixelSpan pv{ img })
        {
            pv.ForEachPixel([clr](auto& px) { PixelFunc::CopyRGB(&px, &clr); });
            img.SetPremultiplied(false);
        }
    }

    /// Fills RGB of all pixels, keeps alpha, then premultiplies
    static void FillRGBAndPremultiply(Image& img, Color clr)
    {
        if (PixelSpan pv{ img })
        {
            pv.ForEachPixel([clr](auto& px)
            {
                PixelFunc::CopyRGB(&px, &clr);
                PixelFunc::Premultiply(px);
            });
            img.SetPremultiplied(true);
        }
    }

    /// Fills alpha of all pixels to the specified value, keeping RGB unchanged
    static void FillAlphaOnly(Image& img, BYTE alpha)
    {
        if (PixelSpan pv{ img })
        {
            pv.ForEachPixel([alpha](auto& px) { px.a = alpha; });
            img.SetPremultiplied(false);
        }
    }

    /// Fills the entire image with the specified color (RGBA)
    static void FillColor(Image& img, Color clr)
    {
        if (PixelSpan pv{ img })
        {
            pv.ForEachPixel([clr](auto& px) { px = clr; });
            img.SetPremultiplied(false);
        }
    }

    /// Pre-multiplies RGB by alpha
    static void Premultiply(Image& img)
    {
        PixelSpan   pv{ img };
        if (!pv || img.IsPremultiplied()) { assert(false); return; }

        pv.ForEachPixel([](auto& px) { PixelFunc::Premultiply(px); });
        img.SetPremultiplied(true);
    }

    static void ApplyBackground(Image& img, Color bk)
    {
        PixelSpan   pv{ img };
        if (!pv || img.IsPremultiplied()) { assert(false); return; }

        pv.ForEachPixel([bk](auto& px)
        {
            Color   up = px;
            px = bk;
            PixelFunc::BlendStraightAlpha(px, up);
        });
    }

public:
    struct PixelSpan
    {
        Color   * begin{};
        Color   * end{};
        bool   use_parallel = false;

        PixelSpan(const Image& img)
        {
            if (img.ColorBits() == 32)
            {
                int   count = img.Width() * img.Height();
                begin = (Color*)img.GetMemStart();
                end = begin + count;
                use_parallel = count > parallel_threshold; // 0.5M pixels
            }
            else { assert(false); }
        }

        explicit operator bool() const { return begin != nullptr; }

        template<typename T>
        void ForEachPixel(T&& func) const
        {
            if (use_parallel)
                std::for_each(std::execution::par, begin, end, func);
            else
                std::for_each(begin, end, func);
        }
    };
};

_PHOXO_NAMESPACE_END
