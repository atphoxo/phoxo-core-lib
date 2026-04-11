#pragma once
#include "base_utils.h"
#include "text_render.h"
#include "codec_gdiplus_save_params.h"

_PHOXO_BEGIN

struct StreamHGlobalView
{
    IStreamPtr   m_stream;
    HGLOBAL   m_global{};
    void*   m_data{};
    SIZE_T   m_size{};

    explicit operator bool() const { return m_data && m_size; }

    StreamHGlobalView(IStream* sp) : m_stream(sp)
    {
        if (sp && SUCCEEDED(::GetHGlobalFromStream(sp, &m_global)) && m_global)
        {
            m_data = GlobalLock(m_global);
            m_size = GlobalSize(m_global);
        }
    }

    ~StreamHGlobalView()
    {
        if (m_data) { GlobalUnlock(m_global); }
    }
};

/// Read / Write image using Gdi+.
class CodecGdiplus
{
public:
    static Image LoadFile(PCWSTR filepath, Gdiplus::PixelFormat output_format = PixelFormat32bppARGB)
    {
        Gdiplus::Bitmap   src(filepath);
        return ImageHandler::Make(src, output_format);
    }

    static Image LoadStream(IStream* sp, Gdiplus::PixelFormat output_format = PixelFormat32bppARGB)
    {
        Gdiplus::Bitmap   src(sp);
        return ImageHandler::Make(src, output_format);
    }

    static bool SaveFile(PCWSTR filepath, const Image& img, int jpeg_quality = 0, int dpi = 0)
    {
        auto   src = GdiplusUtils::CreateBitmapReference(img);
        if (!src)
            return false;

        if (dpi)
            src->SetResolution((float)dpi, (float)dpi);

        internal::GdiplusSaveParams   param(filepath, jpeg_quality);
        return src->Save(filepath, &param.m_type_CLSID, param.m_encoder_param.get()) == Gdiplus::Ok;
    }

    // format can be ".jpg", ".png", etc.
    static IStreamPtr SaveStream(const Image& img, PCWSTR format, int jpeg_quality = 0)
    {
        auto   src = GdiplusUtils::CreateBitmapReference(img);
        if (!src)
            return nullptr;

        IStreamPtr   stream;
        CreateStreamOnHGlobal(NULL, TRUE, &stream);

        internal::GdiplusSaveParams   param(format, jpeg_quality);
        if (src->Save(stream, &param.m_type_CLSID, param.m_encoder_param.get()) == Gdiplus::Ok)
            return stream;

        return nullptr;
    }
};

_PHOXO_NAMESPACE_END
