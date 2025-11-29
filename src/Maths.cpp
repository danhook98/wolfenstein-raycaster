#include "Maths.h"

#include <numbers>
#include <cmath>

namespace maths
{
    float degreesToRadians(const float degrees)
    {
        return degrees * (std::numbers::pi_v<float> / 180.0f);
    }

    float radiansToDegrees(const float radians)
    {
        return radians * (180.0f / std::numbers::pi_v<float>);
    }

    float normaliseAngle(const float angle)
    {
        if (angle <= 0.0f)
            return angle + (2.0f * std::numbers::pi_v<float>);
        else if (angle > (2.0f * std::numbers::pi))
            return angle - (2.0f * std::numbers::pi_v<float>);

        return angle;
    }

    float distanceBetween(const float x1, const float y1, const float x2, const float y2)
    {
        const float x = x2 - x1;
        const float y = y2 - y1;

        return std::sqrt((x * x) + (y * y));
    }
}