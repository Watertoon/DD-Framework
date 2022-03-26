#pragma once

namespace dd::util {

    namespace {
        s64 system_frequency = 0;
        s64 display_frequency = 0;
        s64 target_delta_frequency = 0;
        s64 last_frame_time = 0;
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

    void SetFrameFrequency(HDC device_context) {
        /* Get monitor refresh rate (note; loses precision) */
        const s32 refresh = ::GetDeviceCaps(device_context, VREFRESH);
        DD_ASSERT(refresh != 0 && refresh != 1);

        display_frequency = refresh;
        target_delta_frequency = system_frequency / display_frequency;
    }

    void BeginFrame() {
        last_frame_time = GetSystemTick();
    }
    
    s64 GetMillisecondsFromTick(s64 tick) {
        return ((tick % system_frequency) * dd::util::math::TPow<s64, 10, 3> / system_frequency) + ((tick / system_frequency) * dd::util::math::TPow<s64, 10, 3>);
    }
    
    void WaitUntilNextFrame() {
        const s32 sleep_time = static_cast<s32>(GetMillisecondsFromTick(target_delta_frequency / (GetSystemTick() - last_frame_time)));
        ::Sleep(sleep_time);
    }
}
