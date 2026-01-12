#pragma once

namespace D2D
{
    class SVGDrawer
    {
    private:
        IStreamPtr   m_stream;

    public:
        explicit SVGDrawer(IStream* stream) : m_stream{ stream }
        {
        }

        explicit SVGDrawer(LPCVOID ptr, UINT length) : m_stream{ phoxo::Utils::CreateMemStream(ptr, length) }
        {
        }

        // Render SVG into a premultiplied-alpha WIC bitmap
        bool DrawSvg(IWICBitmap* target, float scale, ID2D1Factory* factory) const
        {
            D2D1_SIZE_F   viewport = GetViewportSize(target);
            if (m_stream && (viewport.width > 0))
            {
                // Requires Windows 10 1703 (15063+) for DeviceContext5, RenderTarget -> DeviceContext5 conversion
                if (ID2D1DeviceContext5Ptr dc5 = CreateWicBitmapRenderTarget(factory, target))
                {
                    if (scale != 1)
                    {
                        dc5->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale));
                    }
                    return DrawSvgDocument(dc5, viewport);
                }
            }
            return false;
        }

    private:
        static D2D1_SIZE_F GetViewportSize(IWICBitmap* target)
        {
            auto   sz = WIC::GetBitmapSize(target);
            return { (float)sz.cx, (float)sz.cy };
        }

        bool DrawSvgDocument(ID2D1DeviceContext5* dc, D2D1_SIZE_F view_size) const
        {
            ID2D1SvgDocumentPtr   svg;
            dc->CreateSvgDocument(m_stream, view_size, &svg);
            if (svg)
            {
                dc->BeginDraw();
                dc->DrawSvgDocument(svg);
                return dc->EndDraw() == S_OK;
            }
            return false;
        }
    };
}
