#pragma once
/// @cond
#include <ShlObj.h>
/// @endcond

_PHOXO_BEGIN
_PHOXO_EFFECT_BEGIN

struct HGlobalUtil
{
    static HGLOBAL Alloc(SIZE_T size)
    {
        return ::GlobalAlloc(GMEM_MOVEABLE, size);
    }

    static HGLOBAL FromMemory(LPCVOID data, SIZE_T size)
    {
        if (!data || !size)
            return NULL;
        HGLOBAL   global = Alloc(size);
        if (!global)
            return NULL;
        void*   ptr = GlobalLock(global);
        if (!ptr)
        {
            GlobalFree(global);
            return NULL;
        }
        memcpy(ptr, data, size);
        GlobalUnlock(global);
        return global;
    }

    static HGLOBAL FromValue(DWORD v) { return FromMemory(&v, sizeof(v)); }
};

/// Copy image to clipboard (32 bit).
class CopyToClipboard : public ImageEffect
{
private:
    const CString   m_filepath;

public:
    CopyToClipboard(PCWSTR filepath = L"") : m_filepath(filepath) {}

private:
    ProcessMode QueryProcessMode() override
    {
        return ProcessMode::EntireMyself;
    }

    static HGLOBAL CreateImageData(const Image& img)
    {
        HGLOBAL   global = HGlobalUtil::Alloc(sizeof(BITMAPINFOHEADER) + img.PixelBufferBytes());
        if (!global)
            return NULL;
        auto*   ptr = (BITMAPINFOHEADER*)GlobalLock(global);
        if (!ptr)
        {
            GlobalFree(global);
            return NULL;
        }

        *ptr = { sizeof(BITMAPINFOHEADER), img.Width(), -img.Height(), 1, 32 }; // header
        memcpy(ptr + 1, img.PixelBase(), img.PixelBufferBytes()); // pixel data

        GlobalUnlock(global);
        return global;
    }

    static void SetClipboardPNG(const Image& img)
    {
        IStreamPtr   stm = CodecGdiplus::SaveStream(img, L".png");
        if (StreamHGlobalView view(stm); view)
        {
            UINT   fmt = ::RegisterClipboardFormat(L"PNG");
            SetClipboardData(fmt, HGlobalUtil::FromMemory(view.m_data, view.m_size));
        }
    }

    HGLOBAL CreateFileObject() const
    {
        if (!PathFileExists(m_filepath))
            return NULL;

        std::basic_string<BYTE>   buf;
        DROPFILES   df{ .pFiles = sizeof(DROPFILES), .fWide = TRUE };
        buf.append((const BYTE*)&df, sizeof(df));
        buf.append((const BYTE*)m_filepath.GetString(), m_filepath.GetLength() * 2);
        buf.append(4, 0); // need double 0

        return HGlobalUtil::FromMemory(buf.data(), buf.size());
    }

    void CopyFileObject() const
    {
        if (HGLOBAL fobj = CreateFileObject())
        {
            SetClipboardData(CF_HDROP, fobj);
            SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), HGlobalUtil::FromValue(DROPEFFECT_COPY));
        }
    }

    void ProcessEntire(Image& img, IProgressListener*) override
    {
        if (!::OpenClipboard(NULL))
            return;

        ::EmptyClipboard();
        SetClipboardData(CF_DIB, CreateImageData(img));
        if (!ImageFastPixel::IsFullyOpaque(img))
        {
            SetClipboardPNG(img);
        }
        CopyFileObject();
        ::CloseClipboard();
    }
};

/// Get image from clipboard.
class GetClipboard : public ImageEffect
{
    bool IsSupported(const Image& img) override { return true; }
    ProcessMode QueryProcessMode() override { return ProcessMode::EntireMyself; }

    bool LoadClipFormat(PCWSTR format, Image& output)
    {
        HGLOBAL   handle = ::GetClipboardData(::RegisterClipboardFormat(format));
        if (!handle)
            return false;

        if (IStreamPtr stm; SUCCEEDED(CreateStreamOnHGlobal(handle, FALSE, &stm)) && stm)
        {
            output = CodecGdiplus::LoadStream(stm);
        }
        return output.IsValid();
    }

    void ProcessEntire(Image& img, IProgressListener*) override
    {
        if (!::OpenClipboard(NULL))
            return;

        if (LoadClipFormat(L"PNG", img))
        {
        }
        else if (LoadClipFormat(L"image/png", img)) // from Inkscape
        {
        }
        else if (auto ddb = (HBITMAP)::GetClipboardData(CF_BITMAP)) // fallback
        {
            IWICBitmapPtr   bmp = WIC::CreateBitmapFromHBITMAP(ddb, WICBitmapIgnoreAlpha);
            img = ImageHandler::Make(bmp, WICNormal32bpp);
        }
        ::CloseClipboard();
    }
};

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
