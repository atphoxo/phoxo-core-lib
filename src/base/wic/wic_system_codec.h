#pragma once

namespace WIC
{
    _PHOXO_INTERNAL_BEGIN
    struct CodecInfo
    {
        CString   m_exts; // such as : ",.jpeg,.jpe,.jpg,.jfif,.exif,"  a comma-wrapped
        GUID   m_format{}; // such as : GUID_ContainerFormatJpeg

        CodecInfo(IWICBitmapCodecInfo& r)
        {
            r.GetContainerFormat(&m_format);

            UINT   read = 0;
            r.GetFileExtensions(0, NULL, &read);
            if (read)
            {
                r.GetFileExtensions(read, m_exts.GetBuffer(read), &read);
                m_exts.ReleaseBuffer();
                m_exts.MakeLower();
                m_exts = L"," + m_exts + L",";
            }
        }
    };

    struct AllDecoderList : public std::vector<CodecInfo>
    {
        AllDecoderList()
        {
            IEnumUnknownPtr   root;
            WIC::g_factory->CreateComponentEnumerator(WICDecoder, WICComponentEnumerateDefault, &root);
            if (!root)
                return;

            IUnknownPtr   unk;
            while (root->Next(1, &unk, NULL) == S_OK)
            {
                if (IWICBitmapCodecInfoPtr ifp = unk)
                {
                    emplace_back(*ifp);
                }
            }
            shrink_to_fit();
        }
    };
    _PHOXO_NAMESPACE_END

    inline GUID GetSystemCodecFormat(PCWSTR filepath)
    {
        static const internal::AllDecoderList   s_decoders;

        CString   ext = PathFindExtension(filepath);
        if (!ext.IsEmpty())
        {
            ext.MakeLower();
            ext = L"," + ext + L",";
            for (const auto& iter : s_decoders)
            {
                if (wcsstr(iter.m_exts, ext))
                    return iter.m_format;
            }
        }
        return GUID_NULL;
    }
}
