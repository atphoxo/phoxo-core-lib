#pragma once

// All functions here depend on the global g_factory
namespace WIC
{
    using namespace phoxo;

    inline IWICImagingFactoryPtr   g_factory;

    inline IWICColorContextPtr GetFirstColorContext(IWICBitmapFrameDecode* frame)
    {
        UINT   count = 0;
        if (frame)
            frame->GetColorContexts(0, NULL, &count);

        if (count)
        {
            IWICColorContextPtr   icc;
            g_factory->CreateColorContext(&icc);
            if (frame->GetColorContexts(1, &icc.GetInterfacePtr(), &count) == S_OK)
                return icc;
        }
        return nullptr;
    }

    inline IWICBitmapSourcePtr ScaleBitmap(IWICBitmapSource* src, CSize dst_size, WICBitmapInterpolationMode mode = WICBitmapInterpolationModeHighQualityCubic)
    {
        // sometimes color error when scaling in PBGRA format (? needs further investigation ?)
        if (GetBitmapSize(src) == dst_size)
            return src;

        IWICBitmapScalerPtr   cmd;
        g_factory->CreateBitmapScaler(&cmd);
        if (cmd && src && (cmd->Initialize(src, dst_size.cx, dst_size.cy, mode) == S_OK))
            return cmd;
        assert(false);
        return src;
    }

    inline IWICBitmapPtr CreateBitmap(CSize image_size, REFWICPixelFormatGUID fmt)
    {
        IWICBitmapPtr   t;
        g_factory->CreateBitmap(image_size.cx, image_size.cy, fmt, WICBitmapCacheOnLoad, &t);
        return t;
    }

    inline IWICBitmapPtr CreateBitmapFromSource(IWICBitmapSource* src)
    {
        IWICBitmapPtr   t;
        if (src) { g_factory->CreateBitmapFromSource(src, WICBitmapCacheOnLoad, &t); }
        return t;
    }

    inline IWICBitmapPtr EnsureBitmapCreated(IWICBitmapSource* src)
    {
        if (IWICBitmapPtr t = src)
            return t;
        return CreateBitmapFromSource(src);
    }

    inline IWICBitmapPtr CreateBitmapFromHBITMAP(HBITMAP src, WICBitmapAlphaChannelOption options)
    {
        IWICBitmapPtr   t;
        if (src) { g_factory->CreateBitmapFromHBITMAP(src, NULL, options, &t); }
        return t;
    }

    inline IWICBitmapFlipRotatorPtr CreateBitmapFlipRotator()
    {
        IWICBitmapFlipRotatorPtr   t;
        g_factory->CreateBitmapFlipRotator(&t);
        return t;
    }

    // an extremely long-running operation caused by the FlipRotator. Caching the source in a bitmap before using FlipRotator will work around the issue.
    inline IWICBitmapSourcePtr FlipRotateBitmapSafe(IWICBitmapSource* src, WICBitmapTransformOptions flag)
    {
        if (IWICBitmapPtr bmp = EnsureBitmapCreated(src))
        {
            auto   cmd = CreateBitmapFlipRotator();
            if (cmd && (cmd->Initialize(bmp, flag) == S_OK))
                return cmd;
        }
        assert(false);
        return src;
    }

    inline IWICBitmapDecoderPtr CreateDecoderFromStream(IStream* stm)
    {
        IWICBitmapDecoderPtr   t;
        if (stm) { g_factory->CreateDecoderFromStream(stm, NULL, WICDecodeMetadataCacheOnDemand, &t); }
        return t;
    }

    inline IWICBitmapDecoderPtr CreateDecoderFromFileNoLock(PCWSTR filepath)
    {
        auto   t = Utils::CreateStreamFromFileNoLock(filepath);
        return CreateDecoderFromStream(t);
    }

    inline bool IsDecoderMissing(REFGUID container)
    {
        IWICBitmapDecoderPtr   test;
        auto   hr = g_factory->CreateDecoder(container, nullptr, &test);
        return (hr == WINCODEC_ERR_COMPONENTNOTFOUND) || (hr == WINCODEC_ERR_COMPONENTINITIALIZEFAILURE);
    }

    inline bool IsEncoderMissing(REFGUID container)
    {
        IWICBitmapEncoderPtr   test;
        auto   hr = g_factory->CreateEncoder(container, nullptr, &test);
        return (hr == WINCODEC_ERR_COMPONENTNOTFOUND) || (hr == WINCODEC_ERR_COMPONENTINITIALIZEFAILURE);
    }
}
