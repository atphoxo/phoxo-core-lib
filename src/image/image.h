#pragma once
#include "image_effect.h"

_PHOXO_BEGIN

/**
    @brief Image object

        The image coordinate system follows the convention where:
    - The origin (0, 0) is located at the <span style='color:#FF0000'>upper-left</span> corner.
    - The **x-axis** increases from left to right.
    - The **y-axis** increases from top to bottom.
*/
class Image final
{
public:
    enum
    {
        PremultipliedAlpha = 0x01,
    };

private:
    int   m_width = 0;
    int   m_height = 0;
    int   m_stride = 0;
    uint16_t   m_attribute = 0;
    uint16_t   m_bpp = 0;
    BYTE*   m_pixel = nullptr;
    HBITMAP   m_DIB_Handle = nullptr;

public:
    /// @name Construction
    /// @{
    Image() {} ///< default constructor

    Image(const Image& img) { *this = img; } ///< copy constructor

    Image(Image&& img) { Swap(img); } ///< move constructor

    ~Image() { Destroy(); } ///< destructor (non-virtual)

    /// copy assignment
    Image& operator=(const Image& img)
    {
        if (&img == this) { assert(false); return *this; } // check self-assignment
        if (!img) { Destroy(); return *this; } // empty source

        if (Create(img.Size(), img.m_bpp, img.m_attribute))
        {
            memcpy(m_pixel, img.m_pixel, img.PixelBufferSize());
        }
        return *this;
    }

    /// move assignment
    Image& operator=(Image&& other)
    {
        if (&other == this) { assert(false); return *this; } // check self-assignment
        Destroy();
        Swap(other);
        return *this;
    }
    /// @}

    /// @name Create / Destroy
    /// @{
    bool Create(SIZE image_size, int bpp = 32, int attribute = 0) { return Create(image_size.cx, image_size.cy, bpp, attribute); }

    /// create a new image, bpp can be <span style='color:#FF0000'>8 , 24 , 32</span>
    bool Create(int width, int height, int bpp = 32, int attribute = 0)
    {
        Destroy();
        if (width <= 0 || height == 0 || bpp <= 0)
        {
            assert(false); return false;
        }

        m_attribute = (uint16_t)attribute;
        m_width = width;
        m_height = abs(height);
        m_bpp = (uint16_t)bpp;
        m_stride = Math::CalcStride(width, bpp);
        AllocPixelBuffer();
        if (!m_pixel || (bpp == 1) || (bpp == 4) || (bpp == 16)) // unsupported format
        {
            Destroy();
            assert(false); return false;
        }
        return true;
    }

    /// destroy the image
    void Destroy()
    {
        if (m_DIB_Handle) { DeleteObject(m_DIB_Handle); }
        InitMember();
    }

    /// swap data with another image
    void Swap(Image& img)
    {
        std::swap(img.m_attribute, m_attribute);
        std::swap(img.m_width, m_width);
        std::swap(img.m_height, m_height);
        std::swap(img.m_bpp, m_bpp);
        std::swap(img.m_stride, m_stride);
        std::swap(img.m_pixel, m_pixel);
        std::swap(img.m_DIB_Handle, m_DIB_Handle);
    }

    /// release ownership of the DIB handle
    HBITMAP Detach()
    {
        auto   bmp = m_DIB_Handle;
        InitMember();
        return bmp;
    }

    /// fill the pixel buffer with a byte value (default 0)
    void ClearBuffer(BYTE v = 0)
    {
        if (m_pixel) { memset(m_pixel, v, PixelBufferSize()); }
    }
    /// @}

    /// @name Attributes
    /// @{
    bool IsValid() const { return m_pixel != 0; }
    bool IsInside(int x, int y) const { return (x >= 0) && (x < m_width) && (y >= 0) && (y < m_height); }
    bool IsInside(POINT pt) const { return IsInside(pt.x, pt.y); }

    /// no boundary checks, so <span style='color:#FF0000'>Crash</span> if y exceed.
    inline BYTE* GetLinePtr(int y) const
    {
        assert(IsInside(0, y));
        return m_pixel + (y * m_stride);
    }
    inline BYTE* GetPixel(int x, int y) const
    {
        assert(IsInside(x, y));
        auto   py = GetLinePtr(y);
        if (m_bpp == 32)
            return (py + x * 4);
        if (m_bpp == 8)
            return (py + x);
        return (py + x * 3); // 24bpp
    }
    BYTE* GetPixel(POINT pt) const { return GetPixel(pt.x, pt.y); }

    CSize Size() const { return { m_width, m_height }; }
    int Width() const { return m_width; }
    int Height() const { return m_height; }
    int ColorBits() const { return m_bpp; }
    int Stride() const { return m_stride; }
    int PixelBufferSize() const { return m_stride * m_height; } ///< buffer size in bytes
    BYTE* GetMemStart() const { return m_pixel; } ///< pointer to pixel buffer
    int Attribute() const { return m_attribute; }
    operator HBITMAP() const { return m_DIB_Handle; }
    explicit operator bool() const { return m_pixel != 0; }

    bool IsPremultiplied() const { return (m_attribute & PremultipliedAlpha); }
    void SetPremultiplied(bool v) { v ? ModifyAttribute(0, PremultipliedAlpha) : ModifyAttribute(PremultipliedAlpha, 0); }
    void ModifyAttribute(int remove, int add)
    {
        m_attribute = (uint16_t)((m_attribute & ~remove) | add);
    }
    /// @}

    /// @name Temporary object
    /// @{

    /// you can apply some effect (e.g. effect::BrightnessContrast) on buffer
    void Attach32bppBuffer(int width, int height, void* pixel)
    {
        assert(!IsValid());
        if ((width > 0) && (height != 0) && pixel)
        {
            m_width = width;
            m_height = abs(height);
            m_bpp = 32;
            m_pixel = (BYTE*)pixel;
            m_stride = Math::CalcStride(width, 32);
        }
        else { assert(false); }
    }
    /// @}

    /// @name Image Processing
    /// @{

    /// apply an effect to the current image, more detail refer to ImageEffect.
    void ApplyEffect(ImageEffect& effect, IProgressListener* progress = nullptr)
    {
        if (!effect.IsSupported(*this)) { assert(false); return; }

        // before
        effect.OnBeforeProcess(*this);
        if (progress)
            progress->UpdateProgress(0);

        if (effect.QueryProcessMode() == ImageEffect::ProcessMode::EntireMyself)
        {
            effect.ProcessEntire(*this, progress);
        }
        else
        {
            if (effect.IsParallelEnabled())
            {
                ApplyEffectParallel(effect, progress);
            }
            else
            {
                effect.ProcessRegion(*this, CRect(0, 0, Width(), Height()), progress);
            }
        }

        effect.OnAfterProcess(*this);
        if (progress && !progress->IsUserCanceled())
            progress->UpdateProgress(100);
    }

    template<typename T>
    void IterateRangePixels(const RECT& rc, T& effect)
    {
        int   bpp = ColorBits() / 8;
        for (int y = rc.top; y < rc.bottom; y++)
        {
            BYTE*   cur = GetPixel(rc.left, y);
            for (int x = rc.left; x < rc.right; x++, cur += bpp)
            {
                T::HandlePixel(*this, x, y, (Color*)cur, effect);
            }
        }
    }
    /// @}

private:
    void InitMember()
    {
        m_attribute = 0; m_width = 0; m_height = 0; m_bpp = 0; m_stride = 0;
        m_pixel = nullptr; m_DIB_Handle = nullptr;
    }

    void AllocPixelBuffer()
    {
        BYTE   buffer[sizeof(BITMAPINFOHEADER) + 16 + 256 * sizeof(RGBQUAD)]{}; // +16 bytes for optional bit masks (used by some 16/32-bpp formats)
        auto   info = (BITMAPINFO*)buffer;

        info->bmiHeader = { sizeof(BITMAPINFOHEADER), m_width, -m_height, 1, m_bpp }; // the height is negative, from top to bottom
        m_DIB_Handle = CreateDIBSection(NULL, info, DIB_RGB_COLORS, (VOID**)&m_pixel, NULL, 0);
    }

    void ApplyEffectParallel(ImageEffect& effect, IProgressListener* progress);
};

_PHOXO_NAMESPACE_END
