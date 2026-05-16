#pragma once

namespace D2D
{
    struct SharedResources
    {
        ID2D1DCRenderTargetPtr   dc_target;
        ID2D1SolidColorBrushPtr  brush;

        SharedResources()
        {
            dc_target = CreateDCRenderTarget(D2D1_RENDER_TARGET_TYPE_SOFTWARE);
            if (dc_target)
            {
                dc_target->CreateSolidColorBrush(D2D1::ColorF(0), &brush);
            }
            assert(dc_target && brush);
        }
    };

    inline std::unique_ptr<SharedResources>   g_shared_res;

    inline SharedResources& GetSharedResources()
    {
        if (!g_shared_res)
            g_shared_res = std::make_unique<SharedResources>();
        return *g_shared_res;
    }

    inline void ResetSharedResources()
    {
        g_shared_res = nullptr;
    }

    inline void UpdateEndDrawResult(HRESULT hr)
    {
        if (hr == D2DERR_RECREATE_TARGET)
        {
            ResetSharedResources();
            return;
        }
        assert(SUCCEEDED(hr));
    }

    inline ID2D1DCRenderTarget* GetSharedDCRenderTarget() { return GetSharedResources().dc_target; }
    inline ID2D1SolidColorBrush* GetSharedSolidBrush() { return GetSharedResources().brush; }
}
