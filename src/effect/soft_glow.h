#pragma once

_PHOXO_BEGIN
_PHOXO_EFFECT_BEGIN

/// Soft glow (32 bit).
class SoftGlow : public PixelIterator<SoftGlow>
{
private:
    Image   m_bak;
    BrightnessContrast   m_bc;
    StackBlur   m_gauss_blur;

public:
    /// radius >= 1 \n -100 <= brightness <= 100 \n -100 <= contrast <= 100.
    SoftGlow(int radius, int brightness, int contrast)
        : m_gauss_blur(radius)
        , m_bc(brightness, contrast)
    {
    }

    static void HandlePixel(Image& img, int x, int y, Color* px, SoftGlow& effect)
    {
        effect.Process(x, y, *px);
    }

private:
    void OnBeforeProcess(Image& img) override
    {
        m_bak = img;

        m_gauss_blur.EnableParallel(IsParallelEnabled());
        img.ApplyEffect(m_gauss_blur);

        m_bc.EnableParallel(IsParallelEnabled());
        img.ApplyEffect(m_bc);
    }

    static void ProcessChannel(BYTE& dst, BYTE origin)
    {
        dst = Math::Clamp0255i(255 - (255 - origin) * (255 - dst) / 255);
    }

    void Process(int x, int y, Color& px) const
    {
        auto   origin = (Color*)m_bak.GetPixel(x, y);
        ProcessChannel(px.r, origin->r);
        ProcessChannel(px.g, origin->g);
        ProcessChannel(px.b, origin->b);
        px.a = origin->a;
    }
};

_PHOXO_NAMESPACE_END
_PHOXO_NAMESPACE_END
