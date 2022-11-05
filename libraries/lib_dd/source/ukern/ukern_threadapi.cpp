#include <dd.hpp>

namespace dd::ukern {

    Result CreateThread(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, u32 priority, u32 core_id) {
        return impl::GetScheduler()->CreateThreadImpl(out_handle, thread_func, arg, stack_size, priority, core_id);
    }

    void ExitThread() {
        impl::GetScheduler()->ExitThreadImpl();
    }

    Result StartThread(UKernHandle handle) {
        return impl::GetScheduler()->SetActivityImpl(handle, ActivityLevel_Schedulable);
    }

    Result ResumeThread (UKernHandle handle) {
        return impl::GetScheduler()->SetActivityImpl(handle, ActivityLevel_Schedulable);
    }

    Result SuspendThread(UKernHandle handle) {
        return impl::GetScheduler()->SetActivityImpl(handle, ActivityLevel_Suspended);
    }

    void SetThreadName(UKernHandle handle, const char *name) {

        /* Get fiber from handle */
        FiberLocalStorage *fiber = impl::GetScheduler()->GetFiberByHandle(handle);

        /* Set name */
        if (name == nullptr) {
            impl::UserScheduler::SetInitialFiberNameUnsafe(fiber);
        } else {
            ::strncpy(fiber->fiber_name_storage, name, MaxFiberNameLength);
        }
    }

    void SetThreadNamePointer(UKernHandle handle, const char *name) {

        /* Get fiber from handle */
        FiberLocalStorage *fiber = impl::GetScheduler()->GetFiberByHandle(handle);

        /* Set name */
        if (name == nullptr) {
            impl::UserScheduler::SetInitialFiberNameUnsafe(fiber);
        } else {
            fiber->fiber_name = name;
        }
    }

    Result SetThreadPriority(UKernHandle handle, u32 priority) {
        return impl::GetScheduler()->SetPriorityImpl(handle, priority);
    }

    Result SetThreadCoreMask(UKernHandle handle, UKernCoreMask core_mask) {
        return impl::GetScheduler()->SetCoreMaskImpl(handle, core_mask);
    }

    void Sleep(TimeSpan timeout_span) {
        impl::GetScheduler()->SleepThreadImpl(impl::GetAbsoluteTimeToWakeup(timeout_span));
    }

    void YieldThread() {
        impl::GetScheduler()->SleepThreadImpl(0);
    }

    ThreadType *GetCurrentThread() { return impl::GetScheduler()->GetCurrentThreadImpl(); }
}
