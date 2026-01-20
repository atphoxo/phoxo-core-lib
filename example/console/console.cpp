#include <iostream>

#include <windows.h>
#include <atltypes.h>
#include <atlstr.h>

#include "../../src/phoxo_core.h"
namespace eff = phoxo::effect;
using phoxo::CodecWIC;
using phoxo::CodecGdiplus;
using phoxo::GdiplusUtils;
using FImage = phoxo::Image;

using namespace Gdiplus;

void ApplyBrightness(const auto& img)
{
    FImage   tmp = img;
    eff::BrightnessContrast   cmd(60, 10);
    cmd.EnableParallel(true);
    tmp.ApplyEffect(cmd);
    CodecGdiplus::SaveFile(L"d:\\phoxo_corelib_test_brightness.jpg", tmp);
    std::cout << "The image with the applied effect has been saved to d:\\phoxo_corelib_test_brightness.jpg." << std::endl;
}

void ApplyBlur(const auto& img)
{
    FImage   tmp = img;
    eff::StackBlur   cmd(32);
    cmd.EnableParallel(true);
    tmp.ApplyEffect(cmd);
    CodecGdiplus::SaveFile(L"d:\\phoxo_corelib_test_blur.jpg", tmp);
    std::cout << "The image with the applied effect has been saved to d:\\phoxo_corelib_test_blur.jpg." << std::endl;
}

void ApplyRotate90(const auto& img)
{
    FImage   tmp = img;
    eff::Rotate90   cmd;
    cmd.EnableParallel(true);
    tmp.ApplyEffect(cmd);
    CodecGdiplus::SaveFile(L"d:\\phoxo_corelib_test_rotate90.jpg", tmp);
    std::cout << "The image with the applied effect has been saved to d:\\phoxo_corelib_test_rotate90.jpg." << std::endl;
}

void DrawOnImage(const auto& img)
{
    FImage   tmp = img;
    auto   gpbmp = GdiplusUtils::CreateBitmapReference(tmp);
    Graphics   gc(gpbmp.get());
    gc.SetSmoothingMode(SmoothingModeAntiAlias);

    // draw a rect on the bitmap using GDI+
    Gdiplus::Font   font(L"Arial", (REAL)64);
    SolidBrush   br(Color::DarkGreen);
    gc.DrawString(L"Welcome to PhoXo!", -1, &font, PointF(30, 20), &br);

    // draw a line with arrow
    GraphicsPath   gp;
    PointF   pts[] = { {-2.5, -2.5}, {0, -0.5}, {2.5, -2.5}, {0, 2.5} };
    gp.AddPolygon(pts, 4);
    CustomLineCap   user_cap(&gp, NULL);
    Pen   pen(Color::DarkGreen, 10);
    pen.SetStartCap(LineCapRound);
    pen.SetCustomEndCap(&user_cap);
    gc.DrawLine(&pen, 10, 100, tmp.Width() - 50, tmp.Height() - 50);

    CodecGdiplus::SaveFile(L"d:\\phoxo_corelib_test_draw.jpg", tmp);
    std::cout << "The image with the applied effect has been saved to d:\\phoxo_corelib_test_draw.jpg." << std::endl;
}

int main()
{
    CString   filepath = L"d:\\test.jpg";
    if (!PathFileExists(filepath))
    {
        std::cout << "The file doesn't exist. Please make sure that d:\\test.jpg exists." << std::endl;
        return 0;
    }

    phoxo::CoreLib::Init();

    FImage   img = CodecWIC::LoadFile(filepath, WICNormal32bpp);
    std::cout << "Width: " << img.Width() << " , Height: " << img.Height() << std::endl;

    ApplyBrightness(img);
    ApplyBlur(img);
    ApplyRotate90(img);
    DrawOnImage(img);

    phoxo::CoreLib::Uninit();
    return 0;
}
