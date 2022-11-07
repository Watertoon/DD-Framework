#include <dd.hpp>

namespace dd::util {

    namespace x64 {

        u64 rdtscp(u32 *frequency) {
            return __builtin_ia32_rdtscp(frequency);
        }

        u64 rdtsc() {
            return __builtin_ia32_rdtsc();
        }
    }

    u32 sSystemFrequency   = 0;
    u64 sMaxTickToTimeSpan = 0xFFFF'FFFF'FFFF'FFFF;

    void InitializeTimeStamp() {
        x64::rdtscp(std::addressof(sSystemFrequency));
        sMaxTickToTimeSpan = 0xFFFF'FFFF'FFFF'FFFF - (0xFFFF'FFFF'FFFF'FFFF % sSystemFrequency);
    }

    u64 GetSystemTick() {
        return x64::rdtsc();
    }

    u32 GetSystemTickFrequency() {
        return sSystemFrequency;
    }
    
    u64 GetMaxTickToTimeSpan() {
        return sMaxTickToTimeSpan;
    }
}
