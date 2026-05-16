/*
    PhoXo Core Lib - Header-only C++20 image library (Windows)

    Copyright (C) 2025  Fu Li
    Author  : Fu Li
    Website : https://www.phoxo.com
    Email   : atphoxo@gmail.com
    License : MIT
*/
#pragma once

#include "base/define.h"

// core
#include "image/image.h"
#include "image/image_effect_parallel.h"
#include "image/post_implement.h"

// image operation helpers
#include "imageops/image_handle.h"
#include "imageops/image_drawer.h"
#include "imageops/image_fast_pixel.h"
#include "imageops/sampling_bilinear.h"
#include "imageops/sampling_area_box.h"

// codec
#include "codec_gdiplus/codec_gdiplus.h"
#include "codec_wic/codec_wic.h"

#include "d2d/render_target.h"
#include "d2d/shared_resources.h"
#include "d2d/svg_drawer.h"

// effect
#include "effect/basic.h"
#include "effect/composite.h"

#include "effect/stack_blur.h"
#include "effect/unsharp_mask.h"
#include "effect/soft_glow.h"

#include "effect/clipboard.h"
#include "effect/resize_gdiplus.h"
#include "effect/resize_wic.h"
#include "effect/text_shadow.h"

_PHOXO_BEGIN
/// Init / Uninit
class CoreLib
{
private:
    static inline ULONG_PTR   m_token = 0;
    static inline HRESULT   m_COM_result = E_INVALIDARG;

public:
    static void Init()
    {
        m_COM_result = ::CoInitialize(0); // init COM

        Gdiplus::GdiplusStartupInput   si;
        Gdiplus::GdiplusStartup(&m_token, &si, NULL); // init GDIPLUS

        WIC::g_factory.CreateInstance(CLSID_WICImagingFactory); // init WIC
        WIC::GetSystemCodecFormat(L"");
    }

    static void InitD2D(ID2D1Factory* d2d)
    {
        D2D::g_factory = d2d;
    }

    static void Uninit()
    {
        D2D::ResetSharedResources();
        D2D::g_factory = nullptr;
        WIC::g_factory = nullptr;
        Gdiplus::GdiplusShutdown(m_token);
        if (SUCCEEDED(m_COM_result))
        {
            ::CoUninitialize();
        }
    }
};
_PHOXO_NAMESPACE_END
