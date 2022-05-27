 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <dd.hpp>

namespace dd::util {

    namespace {
        s64 system_frequency = 0;
        s64 last_frame_time = 0;
        s64 delta_tick = 0;
        double delta_time = 0;
    }

    void InitializeTime() {
        LARGE_INTEGER frequency = {};
        const bool result = ::QueryPerformanceFrequency(std::addressof(frequency));
        DD_ASSERT(result == true);
        system_frequency = frequency.QuadPart;
    }

    s64 GetSystemTickFrequency() {
        return system_frequency;
    }

    s64 GetSystemTick() {
        LARGE_INTEGER tick = {};
        const bool result = ::QueryPerformanceCounter(std::addressof(tick));
        DD_ASSERT(result == true);
        return tick.QuadPart;
    }

    void BeginFrame() {
        const s64 last_last_frame = last_frame_time;
        last_frame_time = GetSystemTick();
        delta_tick = last_frame_time - last_last_frame;
        delta_time = static_cast<double>(delta_tick) / static_cast<double>(system_frequency);
    }

    s64 GetMillisecondsFromTick(s64 tick) {
        return ((tick % system_frequency) * dd::util::math::TPow<s64, 10, 3> / system_frequency) + ((tick / system_frequency) * dd::util::math::TPow<s64, 10, 3>);
    }
    static_assert(dd::util::math::TPow<s64, 10, 3> == 1000);

    double GetDeltaTime() {
        return delta_time;
    }
    
    double CalcFps() {
        return static_cast<double>(system_frequency) / static_cast<double>(delta_tick);
    }
}
