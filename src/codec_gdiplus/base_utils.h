#pragma once
#include "base_path_render.h"

_PHOXO_BEGIN

/// Helper
class GdiplusUtils
{
public:
    static unique_ptr<Gdiplus::Bitmap> CreateBitmapReference(const Image& img)
    {
        if (!img)
        {
            assert(false); return nullptr;
        }

        return make_unique<Gdiplus::Bitmap>(img.Width(), img.Height(), img.Stride(), GetPixelFormat(img), img.PixelBase());
    }

private:
    static Gdiplus::PixelFormat GetPixelFormat(const Image& img)
    {
        switch (img.ColorBits())
        {
            case 8: return PixelFormat8bppIndexed;
            case 24: return PixelFormat24bppRGB;
            case 32: return img.IsPremultiplied() ? PixelFormat32bppPARGB : PixelFormat32bppARGB;
        }
        return PixelFormatUndefined;
    }
};

_PHOXO_NAMESPACE_END
