#include <dd.hpp>

namespace dd::sys {

    namespace {
        constinit util::TypeStorage<SystemManager> sSystemManagerStorage = {};
    }

    ALWAYS_INLINE void InitializeSystemManager() {

        /* Construct system manager */
        util::ConstructAt(sSystemManagerStorage);

        SystemManager *instance = util::GetPointer(sSystemManagerStorage);

        /* Get system info */
        ::GetSystemInfo(std::addressof(instance->system_info));
        DD_ASSERT(instance->system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
        DD_ASSERT(instance->system_info.dwPageSize == 0x1000);

        /* Get active processor count from group 0 */
        instance->active_processor_count = ::GetActiveProcessorCount(0);
        DD_ASSERT(instance->active_processor_count != 0);

        /* Get current process affinity mask */
        const bool result = ::GetProcessAffinityMask(::GetCurrentProcess(), std::addressof(instance->process_core_mask), std::addressof(instance->system_core_mask));
        DD_ASSERT(result != false);
        DD_ASSERT(instance->process_core_mask != 0 && instance->system_core_mask != 0);

        instance->process_core_count = util::CountOneBits64(instance->process_core_mask);
    }

    ALWAYS_INLINE SystemManager *GetSystemManager() { return util::GetPointer(sSystemManagerStorage); }

    ALWAYS_INLINE u32 GetProcessProcessorCount() {
        return GetSystemManager()->process_core_count;
    }

    ALWAYS_INLINE u32 GetCurrentThreadCoreNumber() {
        return ::GetCurrentProcessorNumber();
    }

    ALWAYS_INLINE SYSTEM_INFO *GetSystemInfo() {
        return std::addressof(GetSystemManager()->system_info);
    }
}
