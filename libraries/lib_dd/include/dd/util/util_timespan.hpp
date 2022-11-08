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
#pragma once

namespace dd {

    typedef u64 TickSpan;

    class TimeSpan {
        public:
            static constexpr s64 MaxTime = LLONG_MAX;
            static constexpr s64 MinTime = LLONG_MIN;
        private:
            s64 m_time_ns;
        public:
            constexpr ALWAYS_INLINE TimeSpan() : m_time_ns(0) {/*...*/}
            constexpr ALWAYS_INLINE TimeSpan(s64 time_ns) : m_time_ns(time_ns) {/*...*/}

            static ALWAYS_INLINE TimeSpan FromTick(u64 tick) {

                /* Query frequency and limit */
                const s64 frequency            = util::GetSystemTickFrequency();
                const s64 max_tick_to_timespan = util::GetMaxTickToTimeSpan();

                /* Verify tick to timespan limit */
                if (max_tick_to_timespan < tick)  { return MaxTime; }
                if (tick < -max_tick_to_timespan) { return MinTime; }

                const s64 residual = ((tick % frequency) * 1'000'000'000) / frequency;
                const s64 quotient = (tick / frequency) * 1'000'000'000;

                return TimeSpan(residual + quotient);
            }
            static constexpr ALWAYS_INLINE TimeSpan FromNanoSeconds(s64 time_ns)  { return TimeSpan(time_ns); }
            static constexpr ALWAYS_INLINE TimeSpan FromMicroSeconds(s64 time_us) { return TimeSpan(time_us  * 1'000); }
            static constexpr ALWAYS_INLINE TimeSpan FromMilliSeconds(s64 time_ms) { return TimeSpan(time_ms  * 1'000'000); }
            static constexpr ALWAYS_INLINE TimeSpan FromSeconds(s64 time_sec)     { return TimeSpan(time_sec * 1'000'000'000); }

            static constexpr ALWAYS_INLINE TimeSpan GetTimeLeftOnTarget(s64 tick) {
                if (tick == 0) { return 0; }

                const u64 tick_left = tick - util::GetSystemTick();

                if (tick_left == 0 && tick <= tick_left) { return 0; }

                return FromTick(tick_left);
            }

            ALWAYS_INLINE u64 GetTick() const {
                const s64 frequency = util::GetSystemTickFrequency();
                const s64 seconds   = this->GetSeconds();

                const s64 residual = (((m_time_ns + (seconds * -1'000'000'000)) * frequency) + 999'999'999) / 1'000'000'000;
                const s64 product  = seconds * frequency;
                return residual + product;
            }
            constexpr ALWAYS_INLINE s64 GetNanoSeconds()  const { return m_time_ns; }
            constexpr ALWAYS_INLINE s64 GetMicroSeconds() const { return (m_time_ns / 1'000); }
            constexpr ALWAYS_INLINE s64 GetMilliSeconds() const { return (m_time_ns / 1'000'000); }
            constexpr ALWAYS_INLINE s64 GetSeconds()      const { return (m_time_ns / 1'000'000'000); }
    };
}
