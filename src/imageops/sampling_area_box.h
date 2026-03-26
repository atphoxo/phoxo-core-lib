#pragma once

_PHOXO_BEGIN

/// Sampling helper: computes the weighted average color of a sub pixel area.
/// Strongly recommended to enable AVX optimization; on 400M-pixel images, performance can improve by ~3x.
class SamplingAreaBox
{
private:
    // Overlap length of [i, i+1) and [low, high), used as weight in calculation.
    // Can be 0 when the ratio is 0.5
    static double OverlapLength(double i, double low, double high)
    {
        return (std::max)(0.0, (std::min)(i + 1, high) - (std::max)(i, low));
    }

    struct AreaBox
    {
        double   xlow, ylow, xhigh, yhigh;
        int   left, top, right, bottom; // integer boundaries
        double   xcache[103];

        AreaBox(GPointF pt, double ratio)
        {
            double   radius = 0.5 / ratio;
            xlow  = pt.X - radius;
            xhigh = pt.X + radius;
            ylow  = pt.Y - radius;
            yhigh = pt.Y + radius;

            left   = Math::Floor(xlow);
            right  = Math::Floor(xhigh);
            top    = Math::Floor(ylow);
            bottom = Math::Floor(yhigh);

            assert(right - left + 1 <= 103); // maximum length, ensures ratio is not less than 1%
            FillXCache(xcache);
        }

        void FillXCache(double* buf) const
        {
            for (int x = left; x <= right; x++)
                *buf++ = OverlapLength(x, xlow, xhigh);
        }

        double XWeight(int x) const { return xcache[x - left]; }

        double OverlapY(int y) const { return OverlapLength(y, ylow, yhigh); }
    };

public:
    static Color Get(const Image& img, GPointF pt, double ratio)
    {
        const AreaBox   box(pt, ratio);
        const int   left = box.left, top = box.top, right = box.right, bottom = box.bottom;

        Accumulate   acc;
        for (int y = top; y <= bottom; y++) // Inclusive range: must use <=
        {
            if (y < 0 || y >= img.Height())
                continue;

            double   h = box.OverlapY(y);
            auto   line = (const Color*)img.GetLinePtr(y); // left may be negative
            for (int x = left; x <= right; x++)
            {
                if (img.IsInside(x, y))
                {
                    acc.Sum(line[x], box.XWeight(x) * h);
                }
            }
        }
        return acc.ResultColor();
    }

private:
    struct Accumulate
    {
        double   sb = 0, sg = 0, sr = 0, sa = 0, total_weight = 0;

        void Sum(const Color& px, double weight)
        {
            px.PremulSum(sb, sg, sr, sa, weight);
            total_weight += weight;
        }

        Color ResultColor() const
        {
            Color   ret;
            if (total_weight > 0) // total_weight > 0 if accumulated, 0 if outside the layer
            {
                ret.a = (BYTE)(sa / total_weight + 0.5);
                if (ret.a) // may be fully transparent
                {
                    ret.b = (BYTE)(sb / sa + 0.5);
                    ret.g = (BYTE)(sg / sa + 0.5);
                    ret.r = (BYTE)(sr / sa + 0.5);
                }
            }
            return ret;
        }
    };
};

_PHOXO_NAMESPACE_END
