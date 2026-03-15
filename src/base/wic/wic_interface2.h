#pragma once

/// @cond
class CComPROPVARIANT : public PROPVARIANT
{
public:
    CComPROPVARIANT() { PropVariantInit(this); }
    explicit CComPROPVARIANT(USHORT v) { InitPropVariantFromUInt16(v, this); }
    explicit CComPROPVARIANT(PCWSTR v) { InitPropVariantFromString(v, this); }
    ~CComPROPVARIANT() { PropVariantClear(this); }

    operator LPPROPVARIANT()
    {
        return this;
    }

    int ParseInteger() const
    {
        if (IsVarTypeInteger(vt))
        {
            return PropVariantToInt32WithDefault(*this, 0);
        }
        assert(false);
        return 0;
    }

    CString ParseString() const
    {
        if (vt == VT_LPSTR)  return CString(pszVal);
        if (vt == VT_LPWSTR)  return CString(pwszVal);

        if ((vt == (VT_LPSTR | VT_VECTOR)) && calpstr.cElems)
            return CString(calpstr.pElems[0]);

        assert(false);
        return L"";
    }

    float ParseRational() const
    {
        if (vt == VT_UI8 || vt == VT_I8)
        {
            if (uhVal.HighPart)
                return uhVal.LowPart / (float)uhVal.HighPart;
            else
                return 0; // Some phones report max aperture as 0, prevent repeated asserts
        }
        assert(false);
        return 0;
    }

    float ParseGPSLocation() const
    {
        if (cauh.cElems >= 3)
        {
            int   t1 = (VT_UI8 | VT_VECTOR);
            int   t2 = (VT_I8 | VT_VECTOR);
            if ((vt == t1) || (vt == t2))
            {
                ULONGLONG   buf[3] = { cauh.pElems[0].QuadPart, cauh.pElems[1].QuadPart, cauh.pElems[2].QuadPart };
                auto   ptr = (ULONG*)buf;
                if (!ptr[1] || !ptr[3] || !ptr[5]) { assert(false); return 0; }

                float   d = ptr[0] / (float)ptr[1];
                float   m = ptr[2] / (float)ptr[3];
                float   s = ptr[4] / (float)ptr[5];
                return d + m / 60 + s / 3600;
            }
        }
        assert(false);
        return 0;
    }
};
/// @endcond
