#pragma once

namespace dd::util {

    void InitializeTimeStamp();

    u64 GetSystemTick();
    u32 GetSystemTickFrequency();
    u64 GetMaxTickToTimeSpan();
}
