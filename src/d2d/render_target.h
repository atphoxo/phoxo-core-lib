#pragma once
#include <dwrite.h>
#include <d2d1_3.h>

_COM_SMARTPTR_TYPEDEF(ID2D1Factory, __uuidof(ID2D1Factory));
_COM_SMARTPTR_TYPEDEF(ID2D1DCRenderTarget, __uuidof(ID2D1DCRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1RenderTarget, __uuidof(ID2D1RenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1Bitmap, __uuidof(ID2D1Bitmap));
_COM_SMARTPTR_TYPEDEF(ID2D1DeviceContext, __uuidof(ID2D1DeviceContext));
_COM_SMARTPTR_TYPEDEF(ID2D1DeviceContext5, __uuidof(ID2D1DeviceContext5));
_COM_SMARTPTR_TYPEDEF(ID2D1SvgDocument, __uuidof(ID2D1SvgDocument));
_COM_SMARTPTR_TYPEDEF(ID2D1SolidColorBrush, __uuidof(ID2D1SolidColorBrush));
_COM_SMARTPTR_TYPEDEF(ID2D1PathGeometry, __uuidof(ID2D1PathGeometry));
_COM_SMARTPTR_TYPEDEF(ID2D1GeometrySink, __uuidof(ID2D1GeometrySink));
_COM_SMARTPTR_TYPEDEF(ID2D1TransformedGeometry, __uuidof(ID2D1TransformedGeometry));

// DWrite
_COM_SMARTPTR_TYPEDEF(IDWriteTextFormat, __uuidof(IDWriteTextFormat));
_COM_SMARTPTR_TYPEDEF(IDWriteTextLayout, __uuidof(IDWriteTextLayout));

namespace D2D
{
    using namespace phoxo;

    inline ID2D1FactoryPtr   g_factory;

    _PHOXO_INTERNAL_BEGIN
    inline auto BuildProperties(D2D1_RENDER_TARGET_TYPE type)
    {
        auto   format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
        return D2D1::RenderTargetProperties(type, format);
    }
    _PHOXO_NAMESPACE_END

    inline ID2D1DCRenderTargetPtr CreateDCRenderTarget(D2D1_RENDER_TARGET_TYPE type)
    {
        auto   prop = internal::BuildProperties(type);
        ID2D1DCRenderTargetPtr   ret;
        g_factory->CreateDCRenderTarget(&prop, &ret);
        return ret;
    }

    inline ID2D1RenderTargetPtr CreateWicBitmapRenderTarget(ID2D1Factory* factory, IWICBitmap* bmp)
    {
        auto   prop = internal::BuildProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE);
        ID2D1RenderTargetPtr   ret;
        if (bmp)
        {
            factory->CreateWicBitmapRenderTarget(bmp, &prop, &ret); assert(ret);
        }
        return ret;
    }

    inline ID2D1BitmapPtr CreateBitmapFromWicBitmap(ID2D1RenderTarget* render, IWICBitmapSource* src_bmp)
    {
        auto   src = WIC::ConvertFormat(src_bmp, WICPremultiplied32bpp);
        ID2D1BitmapPtr   ret;
        if (render && src)
        {
            render->CreateBitmapFromWicBitmap(src, &ret);
        }
        assert(ret);
        return ret;
    }

    // DirectWrite utilities
    inline constexpr float   LAYOUT_MAX = 1e6f;

    inline IDWriteTextLayoutPtr CreateTextLayout(
        IDWriteFactory* factory,
        const CString& str,
        PCWSTR fontname,
        float point_size,
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
        bool italic = false,
        int dpi = USER_DEFAULT_SCREEN_DPI,
        D2D_SIZE_F max_layout = { LAYOUT_MAX, LAYOUT_MAX })
    {
        float   ftsize = point_size * dpi / 72.0f;
        auto   itstyle = italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

        IDWriteTextFormatPtr   format;
        IDWriteTextLayoutPtr   layout;
        factory->CreateTextFormat(fontname, NULL, weight, itstyle, DWRITE_FONT_STRETCH_NORMAL, ftsize, L"", &format);
        factory->CreateTextLayout(str, str.GetLength(), format, max_layout.width, max_layout.height, &layout);
        assert(layout);
        return layout;
    }

    inline void DrawTextLayout(
        ID2D1RenderTarget* target,
        POINT pt,
        IDWriteTextLayout* layout,
        const D2D1::ColorF& text_color)
    {
        if (target && layout && (text_color.a > 0))
        {
            ID2D1SolidColorBrushPtr   brush;
            target->CreateSolidColorBrush(text_color, &brush);
            if (brush)
            {
                target->DrawTextLayout({ (float)pt.x, (float)pt.y }, layout, brush);
                return;
            }
        }
        assert(false);
    }

    /// Creates an alpha mask from a text layout.
    /// 1. This mask can be used with ImageFastPixel::FillRGBKeepAlpha to apply color.
    inline Image CreateTextAlphaMask(IDWriteTextLayout* layout)
    {
        DWRITE_TEXT_METRICS   metrics{};
        if (layout)
            layout->GetMetrics(&metrics);

        Image   ret;
        if (ret.Create((int)ceil(metrics.width), (int)ceil(metrics.height)))
        {
            BitmapHDC   memdc(ret);
            if (auto target = CreateDCRenderTarget(D2D1_RENDER_TARGET_TYPE_SOFTWARE))
            {
                target->BindDC(memdc, CRect({}, ret.Size()));
                target->BeginDraw();
                target->Clear({ 0,0,0,0 });
                DrawTextLayout(target, { 0,0 }, layout, D2D1::ColorF::Black);
                target->EndDraw();
            }
        }
        assert(ret);
        return ret;
    }
}
