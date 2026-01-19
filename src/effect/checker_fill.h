#pragma once

_PHOXO_BEGIN
_PHOXO_EFFECT_BEGIN

/// Fill grid (32 bit).
class CheckerFill : public PixelIterator<CheckerFill>
{
private:
    Color   m_cr1, m_cr2;
    int   m_size;

public:
    /// grid_size - width of each grid, >=1
    /// alpha_value - opacity, 0 (transparent) to 255 (opaque)
    CheckerFill(Color cr1, Color cr2, int grid_size, BYTE alpha_value = 0xFF) : m_cr1(cr1), m_cr2(cr2)
    {
        m_size = (std::max)(grid_size, 1);
        m_cr1.a = m_cr2.a = alpha_value;
    }

private:
    void Process(int x, int y, Color& px) const
    {
        int   nX = x / m_size, nY = y / m_size;
        Color   c = ((nX + nY) % 2 == 0) ? m_cr1 : m_cr2;
        PixelFunc::CompositeStraightAlpha(px, c);
    }

public:
    static void HandlePixel(Image&, int x, int y, Color* px, CheckerFill& eff)
    {
        eff.Process(x, y, *px);
    }
};

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
