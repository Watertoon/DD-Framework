#pragma once

namespace dd::util::x64 {

    ALWAYS_INLINE void pause() {
        __builtin_ia32_pause();
    }
}
