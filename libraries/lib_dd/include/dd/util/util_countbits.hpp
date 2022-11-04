#pragma once

namespace dd::util {

    constexpr ALWAYS_INLINE u32 CountOneBits32(u32 value) {
        if constexpr (std::is_constant_evaluated() == true) {
            return std::popcount(value);
        }
        return __builtin_popcount(value);
    }
    constexpr ALWAYS_INLINE u32 CountOneBits64(u64 value) {
        if constexpr (std::is_constant_evaluated() == true) {
            return std::popcount(value);
        }
        return __builtin_popcountll(value);
    }
}
