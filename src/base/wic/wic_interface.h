#pragma once
/// @cond
#include <propvarutil.h>
#pragma comment (lib, "Propsys.lib")
#include <wincodec.h>
#include <Wincodecsdk.h>
#pragma comment (lib, "Windowscodecs.lib")

// MUST be 0: code assumes default rotation is zero in conditional checks.
static_assert(WICBitmapTransformRotate0 == 0);

#define  WICNormal32bpp  GUID_WICPixelFormat32bppBGRA
#define  WICPremultiplied32bpp  GUID_WICPixelFormat32bppPBGRA

_COM_SMARTPTR_TYPEDEF(IWICImagingFactory, __uuidof(IWICImagingFactory));
_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoder, __uuidof(IWICBitmapDecoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder, __uuidof(IWICBitmapEncoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameDecode, __uuidof(IWICBitmapFrameDecode));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode, __uuidof(IWICBitmapFrameEncode));
_COM_SMARTPTR_TYPEDEF(IWICBitmap, __uuidof(IWICBitmap));
_COM_SMARTPTR_TYPEDEF(IWICBitmapSource, __uuidof(IWICBitmapSource));
_COM_SMARTPTR_TYPEDEF(IWICBitmapLock, __uuidof(IWICBitmapLock));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFlipRotator, __uuidof(IWICBitmapFlipRotator));
_COM_SMARTPTR_TYPEDEF(IWICBitmapScaler, __uuidof(IWICBitmapScaler));
_COM_SMARTPTR_TYPEDEF(IWICBitmapCodecInfo, __uuidof(IWICBitmapCodecInfo));
_COM_SMARTPTR_TYPEDEF(IWICColorContext, __uuidof(IWICColorContext));
_COM_SMARTPTR_TYPEDEF(IWICColorTransform, __uuidof(IWICColorTransform));
_COM_SMARTPTR_TYPEDEF(IWICMetadataReader, __uuidof(IWICMetadataReader));
_COM_SMARTPTR_TYPEDEF(IWICMetadataQueryReader, __uuidof(IWICMetadataQueryReader));
_COM_SMARTPTR_TYPEDEF(IWICMetadataQueryWriter, __uuidof(IWICMetadataQueryWriter));
_COM_SMARTPTR_TYPEDEF(IWICMetadataBlockReader, __uuidof(IWICMetadataBlockReader));
_COM_SMARTPTR_TYPEDEF(IWICMetadataBlockWriter, __uuidof(IWICMetadataBlockWriter));
_COM_SMARTPTR_TYPEDEF(IWICFastMetadataEncoder, __uuidof(IWICFastMetadataEncoder));
_COM_SMARTPTR_TYPEDEF(IWICStream, __uuidof(IWICStream));
/// @endcond

namespace WIC
{
    // 超过此尺寸，WIC 会返回有符号整数溢出错误
    inline constexpr int  MAX_BITMAP_PIXELS = 0x7FFFFFFF / 4;

    inline IWICBitmapFrameDecodePtr GetFrame(IWICBitmapDecoder* decoder, UINT index)
    {
        IWICBitmapFrameDecodePtr   t;
        if (decoder) { decoder->GetFrame(index, &t); }
        return t;
    }

    inline CSize GetBitmapSize(IWICBitmapSource* src)
    {
        UINT   x = 0, y = 0;
        if (src) { src->GetSize(&x, &y); }
        return CSize(x, y);
    }

    inline IWICBitmapSourcePtr ConvertFormat(IWICBitmapSource* src, REFWICPixelFormatGUID dst_format)
    {
        if (!src)
            return nullptr;

        WICPixelFormatGUID   current{};
        src->GetPixelFormat(&current);
        if (current == dst_format)
            return src;

        IWICBitmapSourcePtr   t;
        ::WICConvertBitmapSource(dst_format, src, &t); assert(t);
        return t;
    }
}
