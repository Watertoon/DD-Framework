#include <dd.hpp>

namespace dd::util {
    
    namespace {
        s64 system_frequency = 0;
        s64 display_frequency = 0;
        s64 target_frame_frequency = 0;
        s64 last_frame_time = 0;
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

    void SetFrameFrequency(s32 refresh) {
        display_frequency = refresh;
        target_frame_frequency = system_frequency / display_frequency;
    }

    void BeginFrame() {
        const s64 last_last_frame = last_frame_time;
        last_frame_time = GetSystemTick();
        delta_time = static_cast<double>(last_frame_time - last_last_frame) / static_cast<double>(target_frame_frequency);
    }

    s64 GetMillisecondsFromTick(s64 tick) {
        return ((tick % system_frequency) * dd::util::math::TPow<s64, 10, 3> / system_frequency) + ((tick / system_frequency) * dd::util::math::TPow<s64, 10, 3>);
    }
    static_assert(dd::util::math::TPow<s64, 10, 3> == 1000);

    void WaitUntilNextFrame() {
        const s32 sleep_time = static_cast<s32>(GetMillisecondsFromTick(target_frame_frequency / (GetSystemTick() - last_frame_time)));
        ::Sleep(sleep_time);
    }

    s64 GetTickUntilNextFrame() {
        return static_cast<s64>(target_frame_frequency / (GetSystemTick() - last_frame_time));
    }

    double GetDeltaTime() {
        return delta_time;
    }
}