#include "Maths.h"

#include <numbers>

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
}
