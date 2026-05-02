#pragma once

_PHOXO_BEGIN

enum class ColorChannel
{
    None = 0,
    Red = 1 << 0,
    Green = 1 << 1,
    Blue = 1 << 2,
    Alpha = 1 << 3,
    RGB = Red | Green | Blue,
    RGBA = Red | Green | Blue | Alpha
};

/// 32-bit BGRA color, compatible with RGBQUAD
union Color
{
public:
    RGBQUAD    quad;
    struct { BYTE b, g, r, a; };
    uint32_t   val;

public:
    constexpr Color() : val(0) {}

    explicit constexpr Color(RGBQUAD c) : quad(c) {}
    explicit constexpr Color(int32_t c) : val(c) {}
    explicit constexpr Color(uint32_t c) : val(c) {}

    /// Initialize using Gdiplus: Color c(Gdiplus::Color::Red);
    /// Initialize using D2D: Color c(D2D1::ColorF::Red, 0xFF);
    constexpr Color(int32_t c, int alpha) : val(c)
    {
        a = (BYTE)alpha;
    }

    constexpr Color(int red, int green, int blue, int alpha = 0xFF) : r((BYTE)red), g((BYTE)green), b((BYTE)blue), a((BYTE)alpha)
    {
    }

    operator RGBQUAD() const
    {
        return quad;
    }

    operator Gdiplus::Color() const
    {
        return Gdiplus::Color(a, r, g, b);
    }

    void operator=(uint32_t c) { val = c; }
    void operator=(const RGBQUAD& c) { quad = c; }

    bool operator==(const Color& c) const { return val == c.val; }
    bool operator!=(const Color& c) const { return val != c.val; }

    bool IsColorLight() const
    {
        return (r * 299 + g * 587 + b * 114) > (178 * 1000);
    }

    COLORREF ToCOLORREF() const
    {
        return RGB(r, g, b);
    }

    /// add premultiplied RGBA to accumulators
    void PremulSum(double& sb, double& sg, double& sr, double& sa, double coef) const
    {
        //  sb += (b * a) * coef;
        //  sg += (g * a) * coef;
        //  sr += (r * a) * coef;
        //  sa += a * coef;

        // the code below offers slightly better performance.
        double   ac = a * coef;
        sb += b * ac;
        sg += g * ac;
        sr += r * ac;
        sa += ac;
    }

    static Color FromCOLORREF(COLORREF c)
    {
        return Color(GetRValue(c), GetGValue(c), GetBValue(c));
    }

    /// px is 24 or 32 bpp format.
    static BYTE GetGrayscale(const Color* px)
    {
        return (BYTE)((30 * px->r + 59 * px->g + 11 * px->b) / 100);
    }
};

_PHOXO_NAMESPACE_END
