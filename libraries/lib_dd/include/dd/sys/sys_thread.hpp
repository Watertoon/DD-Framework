#pragma once

namespace dd::sys {

    class Thread : public ThreadBase {
        private:
            ukern::UKernHandle m_thread_handle;
        public:
            Thread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_thread_handle(0) {

                /* Create a thread on the default core */
                const Result result0 = ukern::CreateThread(std::addressof(m_thread_handle), ThreadBase::InternalThreadMain, reinterpret_cast<uintptr_t>(this), stack_size, priority, -1);
                DD_ASSERT(result0 == ResultSuccess);

                ukern::SetThreadName(m_thread_handle, name);
            }

            virtual void StartThread() {
                const Result result0 = dd::ukern::StartThread(m_thread_handle);
                DD_ASSERT(result0 == ResultSuccess);
            }
            virtual void WaitForThreadExit() {
                dd::ukern::ExitThread(m_thread_handle);
            }
            virtual void ResumeThread() {
                const Result result0 = dd::ukern::ResumeThread(m_thread_handle);
                DD_ASSERT(result0 == ResultSuccess);
            }
            virtual void SuspendThread() {
                const Result result0 = dd::ukern::SuspendThread(m_thread_handle);
                DD_ASSERT(result0 == ResultSuccess);
            }

            virtual void SetPriority(s32 priority) {
                const Result result0 = dd::ukern::SetThreadPriority(m_thread_handle, priority);
                DD_ASSERT(result0 == ResultSuccess);
                m_priority = priority;
            }
            virtual void SetCoreMask(u64 core_mask) {
                const Result result0 = dd::ukern::SetThreadCoreMask(m_thread_handle, core_mask);
                DD_ASSERT(result0 == ResultSuccess);
                m_core_mask = core_mask;
            }
    };
}
