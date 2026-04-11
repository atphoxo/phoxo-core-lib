#pragma once

_PHOXO_NAMESPACE(WIC)

class ImageEncoder
{
private:
    const GUID   m_image_format{};
    IWICBitmapEncoderPtr   m_encoder;
    IStreamPtr   m_stream;
    IWICBitmapFrameEncodePtr   m_frame_encode;

private:
    bool InitCreateEncoder()
    {
        [[maybe_unused]] HRESULT hr = g_factory->CreateEncoder(m_image_format, NULL, &m_encoder);
        if (!m_encoder)
        {
            assert(hr == WINCODEC_ERR_COMPONENTNOTFOUND); // Unsupported image encoder format
        }
        return m_encoder != nullptr;
    }

    bool InitStreamFromFile(PCWSTR filepath)
    {
        IWICStreamPtr   tmp;
        g_factory->CreateStream(&tmp);
        m_stream = tmp;
        return (tmp->InitializeFromFilename(filepath, GENERIC_WRITE) == S_OK); // Fails if file is write-protected
    }

public:
    explicit ImageEncoder(REFGUID image_format, int jpeg_quality = 80) : m_image_format{ image_format }
    {
        if (!InitCreateEncoder())
            return;
        CreateStreamOnHGlobal(NULL, TRUE, &m_stream);

        m_encoder->Initialize(m_stream, WICBitmapEncoderNoCache);
        CreateFrameEncode(jpeg_quality);
    }

    explicit ImageEncoder(PCWSTR filepath, int jpeg_quality = 80) : m_image_format{ GetSystemCodecFormat(filepath) }
    {
        if (!InitCreateEncoder())
            return;
        if (!InitStreamFromFile(filepath))
            return;

        m_encoder->Initialize(m_stream, WICBitmapEncoderNoCache);
        CreateFrameEncode(jpeg_quality);
    }

    bool IsEncoderAvailable() const { return m_frame_encode != NULL; }
    bool IsJPEG() const { return m_image_format == GUID_ContainerFormatJpeg; }

    /*    void CopyMetadata(IWICBitmapFrameDecodePtr source_meta)
        {
            IWICMetadataBlockReaderPtr   reader = source_meta;
            IWICMetadataBlockWriterPtr   writer = m_frame_encode;
            if (writer && reader)
            {
                writer->InitializeFromBlockReader(reader);

                IWICMetadataQueryWriterPtr   orient;
                m_frame_encode->GetMetadataQueryWriter(&orient);
                WIC::OrientationTag::Write(orient, 1); // clear orientation tag
            }
        }*/

    void SetICC(IWICColorContext* icc)
    {
        if (m_frame_encode && icc && IsICCSaveSupported(icc))
        {
            m_frame_encode->SetColorContexts(1, &icc);
        }
    }

    bool Write(IWICBitmapSource* src)
    {
        try
        {
            HRESULT hr = m_frame_encode->WriteSource(src, NULL);
            if (FAILED(hr)) { assert(false); return false; }

            hr = m_frame_encode->Commit();
            if (FAILED(hr)) { assert(false); return false; }

            hr = m_encoder->Commit();
            if (FAILED(hr)) { assert(false); return false; }

            return true;
        }
        catch (_com_error&)
        {
            assert(false); return false;
        }
    }

    IStream* GetEncodedStream() const
    {
        return m_stream;
    }

private:
    bool IsICCSaveSupported(IWICColorContext* icc) const
    {
        if (icc)
        {
            if (IsJPEG()) // jpeg支持所有类型icc
                return true;

            if ((m_image_format == GUID_ContainerFormatPng) || (m_image_format == GUID_ContainerFormatTiff))
            {
                // png不支持exif类型，如果设置最后WriteSource会失败，tiff支持exif类型，安全起见先不支持了
                UINT   len = 0;
                icc->GetProfileBytes(0, NULL, &len);
                return (len != 0);
            }
        }
        return false;
    }

    void SetOrientationTag(int orientation)
    {
        if (m_frame_encode)
        {
            IWICMetadataQueryWriterPtr   writer;
            m_frame_encode->GetMetadataQueryWriter(&writer);
            OrientationTag::Write(writer, orientation);
        }
    }

    void CreateFrameEncode(int jpeg_quality)
    {
        try
        {
            if (IsJPEG() || (m_image_format == GUID_ContainerFormatJpegXL))
            {
                IPropertyBag2Ptr   prop;
                m_encoder->CreateNewFrame(&m_frame_encode, &prop);
                WriteProperty(prop, L"ImageQuality", (float)jpeg_quality / 100.0f);
                m_frame_encode->Initialize(prop);

                // if there is no orientation tag, fast rotation of JPEG may fail later
                if (IsJPEG())
                    SetOrientationTag(1);
            }
            else if (m_image_format == GUID_ContainerFormatBmp)
            {
                IPropertyBag2Ptr   prop;
                m_encoder->CreateNewFrame(&m_frame_encode, &prop);
                WriteProperty(prop, L"EnableV5Header32bppBGRA", VARIANT_TRUE, VT_BOOL);
                m_frame_encode->Initialize(prop);
            }
            else
            {
                m_encoder->CreateNewFrame(&m_frame_encode, nullptr);
                m_frame_encode->Initialize(nullptr);
            }
        }
        catch (_com_error&) { assert(false); }
        assert(IsEncoderAvailable());
    }

    template<typename T>
    static void WriteProperty(IPropertyBag2* prop, PCWSTR propname, T value, VARTYPE type = VT_EMPTY)
    {
        _variant_t   val(value);
        if (type == VT_BOOL)
            val.vt = type;

        PROPBAG2   str = { .pstrName = const_cast<PWSTR>(propname) }; // requires writable pointer
        if (prop) { prop->Write(1, &str, &val); }
    }
};

_PHOXO_NAMESPACE_END
