#pragma once

_PHOXO_BEGIN

/// Memory DC with auto bitmap selection.
class BitmapHDC
{
private:
    HDC      m_dc = CreateCompatibleDC(NULL);
    HGDIOBJ  m_bmp_bak;
    HGDIOBJ  m_font_bak;

public:
    /// create DC and select bitmap.
    BitmapHDC(HBITMAP bmp)
    {
        m_bmp_bak = SelectObject(m_dc, bmp); assert(m_bmp_bak);
        m_font_bak = GetCurrentObject(m_dc, OBJ_FONT);
        SetBkMode(m_dc, TRANSPARENT);
        SetStretchBltMode(m_dc, COLORONCOLOR);
    }

    /// restore objects and delete DC.
    ~BitmapHDC()
    {
        SelectObject(m_dc, m_font_bak);
        SelectObject(m_dc, m_bmp_bak);
        DeleteDC(m_dc);
    }

    /// get HDC handle.
    operator HDC() const
    {
        return m_dc;
    }

    BitmapHDC(const BitmapHDC&) = delete;
};

_PHOXO_NAMESPACE_END
