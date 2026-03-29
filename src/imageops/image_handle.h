#pragma once

_PHOXO_BEGIN

/// Handles basic image operations.
class ImageHandler
{
public:
    /// @name Crop & Overlay
    /// @{

    /// copy region from img to output. rect must be fully inside src.
    static Image GetRegion(const Image& img, CRect rect_on_image)
    {
        CRect   rc;
        rc.IntersectRect(CRect(0, 0, img.Width(), img.Height()), rect_on_image);

        Image   output;
        if (!rc.IsRectEmpty() &&
            (rc == rect_on_image) &&
            output.Create(rc.Size(), img.ColorBits(), img.Attribute()))
        {
            int   copy_bytes = output.Width() * img.ColorBits() / 8;
            for (int y = 0; y < output.Height(); y++)
            {
                memcpy(output.GetLinePtr(y), img.GetPixel(rc.left, rc.top + y), copy_bytes);
            }
        }
        else
        {
            assert(false);
        }
        return output;
    }

    /// cover image.
    static void Cover(Image& bottom, const Image& top, POINT pt_on_bottom)
    {
        CRect   rect_top(pt_on_bottom, top.Size());
        CRect   rc;
        rc.IntersectRect(CRect(CPoint(), bottom.Size()), rect_top);

        if (!rc.IsRectEmpty() && (bottom.ColorBits() == top.ColorBits()) && (bottom.Attribute() == top.Attribute()))
        {
            int   copy_bytes = rc.Width() * top.ColorBits() / 8;
            for (int y = rc.top; y < rc.bottom; y++)
            {
                auto   dst = bottom.GetPixel(rc.left, y);
                auto   src = top.GetPixel(rc.left - pt_on_bottom.x, y - pt_on_bottom.y);
                memcpy(dst, src, copy_bytes);
            }
        }
        else
        {
            assert(false);
        }
    }
    /// @}

    /// @name Create Image
    /// @{

    /// Creates an Image from a gdiplus bitmap.
    static Image Make(Gdiplus::Bitmap& src, Gdiplus::PixelFormat output_format)
    {
        using namespace Gdiplus;

        int   attr = (output_format == PixelFormat32bppPARGB) ? Image::PremultipliedAlpha : 0;
        int   bpp = GetPixelFormatSize(output_format);
        UINT   width = src.GetWidth(), height = src.GetHeight();

        Image   img;
        if (img.Create(width, height, bpp, attr))
        {
            BitmapData   bd{ width, height, img.Stride(), output_format, img.PixelBase() };
            Rect   rgn(0, 0, width, height);
            if (src.LockBits(&rgn, ImageLockModeRead | ImageLockModeUserInputBuf, output_format, &bd) == Ok)
            {
                src.UnlockBits(&bd);
            }
        }
        return img;
    }

    /// Creates an Image from a WIC bitmap.
    static Image Make(IWICBitmapSource* src_bmp, REFWICPixelFormatGUID output_format)
    {
        int   attr = (output_format == WICPremultiplied32bpp) ? Image::PremultipliedAlpha : 0;
        int   bpp = GetBitsPerPixel(output_format);
        auto    src = WIC::ConvertFormat(src_bmp, output_format);
        CSize   sz = WIC::GetBitmapSize(src);

        if (Image img; img.Create(sz, bpp, attr))
        {
            // 如果没装hevc ext, heif文件CopyPixels返回0xc00d5212 (Unsupported File Format)
            if (src->CopyPixels(NULL, img.Stride(), img.PixelBufferBytes(), img.PixelBase()) == S_OK)
                return img;
            assert(false);
        }
        return {};
    }
    /// @}

private:
    static int GetBitsPerPixel(REFWICPixelFormatGUID fmt)
    {
        // 目前只用到了 WICNormal32bpp / WICPremultiplied32bpp
        if ((fmt == WICNormal32bpp) ||
            (fmt == WICPremultiplied32bpp) ||
            (fmt == GUID_WICPixelFormat32bppBGR))
            return 32;
        if (fmt == GUID_WICPixelFormat24bppBGR)
            return 24;
        assert(false);
        return 0;
    }
};

_PHOXO_NAMESPACE_END
