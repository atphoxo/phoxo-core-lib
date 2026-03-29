#pragma once
/// @cond
#include <ShlObj.h>
/// @endcond

_PHOXO_BEGIN
_PHOXO_EFFECT_BEGIN

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

    static HGLOBAL CreateImageData(Image& img)
    {
        auto   mem = GlobalAlloc(GMEM_MOVEABLE, img.PixelBufferBytes() + sizeof(BITMAPINFOHEADER));
        auto   ptr = (BITMAPINFOHEADER*)GlobalLock(mem);
        *ptr = { sizeof(*ptr), img.Width(), img.Height(), 1, (WORD)img.ColorBits() };

        Image   tmp;
        tmp.Attach32bppBuffer(img.Width(), img.Height(), ptr + 1);

        CopyMemory(tmp.PixelBase(), img.PixelBase(), img.PixelBufferBytes());
        Flip   eff;
        eff.EnableParallel();
        tmp.ApplyEffect(eff);

        GlobalUnlock(mem);
        return mem;
    }

    HGLOBAL CreateFileObject() const
    {
        if (!PathFileExists(m_filepath))
            return NULL;

        DROPFILES   dp_files = { .pFiles = sizeof(DROPFILES), .fWide = TRUE };

        int   path_bytes = m_filepath.GetLength() * 2 + 2;
        auto   mem = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, sizeof(DROPFILES) + path_bytes + 2); // double 0
        auto   ptr = (BYTE*)GlobalLock(mem);
        *(DROPFILES*)ptr = dp_files;
        ptr += sizeof(DROPFILES);
        CopyMemory(ptr, (PCWSTR)m_filepath, path_bytes);
        GlobalUnlock(mem);
        return mem;
    }

    void CopyFileObject() const
    {
        auto   fobj = CreateFileObject();
        if (!fobj)
            return;

        auto   op = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
        *(DWORD*)GlobalLock(op) = DROPEFFECT_COPY;
        GlobalUnlock(op);

        SetClipboardData(CF_HDROP, fobj);
        SetClipboardData(RegisterClipboardFormat(L"Preferred DropEffect"), op);
    }

    void ProcessEntire(Image& img, IProgressListener*) override
    {
        if (::OpenClipboard(NULL))
        {
            ::EmptyClipboard();
            ::SetClipboardData(CF_DIB, CreateImageData(img));
            CopyFileObject();
            ::CloseClipboard();
        }
    }
};

/// Get image from clipboard.
class GetClipboard : public ImageEffect
{
    bool IsSupported(const Image& img) override
    {
        return true;
    }

    ProcessMode QueryProcessMode() override
    {
        return ProcessMode::EntireMyself;
    }

    void ProcessEntire(Image& img, IProgressListener*) override
    {
        if (::OpenClipboard(NULL))
        {
            IWICBitmapPtr   bmp = WIC::CreateBitmapFromHBITMAP((HBITMAP)::GetClipboardData(CF_BITMAP), WICBitmapUseAlpha);
            img = ImageHandler::Make(bmp, WICNormal32bpp);
            ::CloseClipboard();
        }
    }
};

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
