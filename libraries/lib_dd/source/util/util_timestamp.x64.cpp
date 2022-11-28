 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#include <dd.hpp>

namespace dd::util {

    namespace x64 {

        NO_INLINE u64 rdtscp(u32 *frequency) {
            u64 low = 0;
            u64 high = 0;
            u32 freq = 0;
            asm volatile ("rdtscp\n" : "=a" (low), "=d" (high), "=c" (freq) : : );
            *frequency = freq;
            return (high << 32) + low;
        }

        NO_INLINE u64 rdtsc() {
            return __builtin_ia32_rdtsc();
        }
    }

    u64 sSystemFrequency   = 0;
    u64 sMaxTickToTimeSpan = 0xFFFF'FFFF'FFFF'FFFF;

    void InitializeTimeStamp() {
        const bool result = ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(std::addressof(sSystemFrequency)));
        DD_ASSERT(result == true);
        sMaxTickToTimeSpan = 0xFFFF'FFFF'FFFF'FFFF - (0xFFFF'FFFF'FFFF'FFFF % sSystemFrequency);
    }

    u64 GetSystemTick() {
        u64 time = 0;
        const bool result = ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(std::addressof(time)));
        DD_ASSERT(result == true);
        return time;
    }

    u32 GetSystemTickFrequency() {
        return sSystemFrequency;
    }
    
    u64 GetMaxTickToTimeSpan() {
        return sMaxTickToTimeSpan;
    }
}
