#pragma once

_PHOXO_BEGIN
_PHOXO_INTERNAL_BEGIN

/// @cond
struct GdiplusSaveParams
{
    CLSID   m_type_CLSID;
    ULONG   m_jpeg_quality;
    unique_ptr<Gdiplus::EncoderParameters>   m_encoder_param;

    GdiplusSaveParams(PCWSTR filepath, int jpeg_quality = 0)
    {
        auto   imgtype = ImageFileExtParser::GetType(filepath);
        m_type_CLSID = GetEncoderClsid(imgtype);
        m_jpeg_quality = jpeg_quality;

        if ((imgtype == ImageFormat::Jpeg) && jpeg_quality)
        {
            m_encoder_param = make_unique<Gdiplus::EncoderParameters>();
            m_encoder_param->Count = 1;
            m_encoder_param->Parameter[0] = { Gdiplus::EncoderQuality, 1, Gdiplus::EncoderParameterValueTypeLong, &m_jpeg_quality };
        }
    }

private:
    static GUID GetFormatGUID(ImageFormat fmt)
    {
        using enum ImageFormat;
        using namespace Gdiplus;
        switch (fmt)
        {
            case Bmp: return ImageFormatBMP;
            case Jpeg: return ImageFormatJPEG;
            case Gif: return ImageFormatGIF;
            case Tiff: return ImageFormatTIFF;
            case Png: return ImageFormatPNG;
        }
        return GUID_NULL;
    }

    static CLSID GetEncoderClsid(ImageFormat imgtype)
    {
        using namespace Gdiplus;

        UINT   num = 0, bufsize = 0;
        GetImageEncodersSize(&num, &bufsize);
        if (num && bufsize)
        {
            std::vector<BYTE>   tempbuf(bufsize);
            auto   info = (ImageCodecInfo*)tempbuf.data();
            GetImageEncoders(num, bufsize, info);

            GUID   fmtid = GetFormatGUID(imgtype);
            for (UINT i = 0; i < num; i++)
            {
                if (info[i].FormatID == fmtid)
                {
                    return info[i].Clsid;
                }
            }
        }
        return GUID_NULL;
    }
};
/// @endcond

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
