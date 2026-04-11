#pragma once

_PHOXO_BEGIN

struct HGlobalFactory
{
    static HGLOBAL Alloc(SIZE_T size, UINT flags = GMEM_MOVEABLE)
    {
        return ::GlobalAlloc(flags, size);
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

struct StreamHGlobalView
{
    HGLOBAL  m_global{};
    void*    m_data{};
    SIZE_T   m_size{};

    explicit operator bool() const { return m_data && m_size; }

    StreamHGlobalView(IStream* stm)
    {
        if (stm && SUCCEEDED(::GetHGlobalFromStream(stm, &m_global)) && m_global)
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

_PHOXO_NAMESPACE_END
