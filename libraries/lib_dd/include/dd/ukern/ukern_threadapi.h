#pragma once

namespace dd::ukern {

    namespace impl {
        class UserScheduler;

        UserScheduler *GetScheduler();
    }

    Result CreateThread(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, u32 priority, u32 core_id);
    void   ExitThread();

    Result StartThread(UKernHandle handle);
    Result ResumeThread (UKernHandle handle);
    Result SuspendThread(UKernHandle handle);

    void   SetThreadName(UKernHandle handle, const char *name);
    void   SetThreadNamePointer(UKernHandle handle, const char *name);
    Result SetThreadPriority(UKernHandle handle, u32 priority);
    Result SetThreadCoreMask(UKernHandle handle, UKernCoreMask core_mask);

    void Sleep(TimeSpan timeout_span);
    void YieldThread();

    ThreadType *GetCurrentThread();
}
