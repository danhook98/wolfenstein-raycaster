#pragma once

#include <SDL3/SDL_timer.h>

namespace util
{
    class DeltaClock
    {
    public:
        DeltaClock()
        {
            curr = SDL_GetPerformanceCounter();
            freq = static_cast<double>(SDL_GetPerformanceFrequency());
        }

        double tick()
        {
            last = curr;
            curr = SDL_GetPerformanceCounter();

            return ((curr - last) / freq);
        }

    private:
        Uint64 curr;
        Uint64 last{0};
        double freq;
    };
}
