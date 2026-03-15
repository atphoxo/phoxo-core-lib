#pragma once

namespace WIC
{
    class MetadataIterator
    {
    private:
        std::deque<IWICMetadataReaderPtr>   m_waiting;

    public:
        virtual ~MetadataIterator() = default;

        /// Calling this function in a constructor is prohibited because of callbacks during enum
        void EnumAllMetadata(IWICBitmapFrameDecode* frame_decode)
        {
            AddRootFrameReaders(frame_decode);
            while (m_waiting.size())
            {
                auto   reader = m_waiting.front();
                m_waiting.pop_front();
                EnumSingleReader(reader);
            }
        }

    protected:
        /// meta_format - GUID_MetadataFormatIfd, GUID_MetadataFormatExif ...
        virtual void OnBeforeEnumReader(IWICMetadataReader* reader, REFGUID meta_format) {}

        virtual void OnEnumMetadataItem(REFGUID meta_format, const CComPROPVARIANT& item_id, const CComPROPVARIANT& val) {}

    private:
        void EnumSingleReader(IWICMetadataReader* reader)
        {
            UINT   item_count = GetMetadataItemCount(reader);
            if (!item_count)
                return;

            GUID   meta_format = GetMetadataFormat(reader);
            OnBeforeEnumReader(reader, meta_format);

            for (UINT i = 0; i < item_count; i++)
            {
                CComPROPVARIANT   id, val;
                reader->GetValueByIndex(i, NULL, &id, &val);
                OnEnumMetadataItem(meta_format, id, val);

                if (val.vt == VT_UNKNOWN)
                {
                    IWICMetadataReaderPtr   child = val.punkVal; assert(child);
                    m_waiting.push_back(child);
                }
            }
        }

        void AddRootFrameReaders(IWICBitmapFrameDecode* frame_decode)
        {
            IWICMetadataBlockReaderPtr   block = frame_decode;
            UINT   count = GetFrameMetaReaderCount(block);
            for (UINT i = 0; i < count; i++)
            {
                IWICMetadataReaderPtr   t;
                block->GetReaderByIndex(i, &t);
                m_waiting.push_back(t); // Null pointers will be verified at usage
            }
        }

        static UINT GetFrameMetaReaderCount(IWICMetadataBlockReader* frame)
        {
            UINT   t = 0;
            if (frame) { frame->GetCount(&t); }
            return t;
        }

        static UINT GetMetadataItemCount(IWICMetadataReader* reader)
        {
            UINT   t = 0;
            if (reader) { reader->GetCount(&t); }
            return t;
        }

        static GUID GetMetadataFormat(IWICMetadataReader* reader)
        {
            GUID   t = {};
            if (reader) { reader->GetMetadataFormat(&t); }
            return t;
        }
    };
}
