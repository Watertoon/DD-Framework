#pragma once

namespace dd::sys {

	class ServiceThread : public sys::ThreadBase {
		private:
			HANDLE            m_handle;
            long unsigned int m_win32_thread_id;
            char              m_name[ukern::MaxFiberNameLength];
		public:
			ServiceThread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_handle(nullptr), m_name('\0') {

                /* Create win32 thread */
                m_handle = ::CreateThread(nullptr, stack_size, ThreadBase::InternalServiceThreadMain, this, CREATE_SUSPENDED, std::addressof(m_win32_thread_id));
                DD_ASSERT(m_handle != nullptr);

                /* Set priority */
                bool result0 = ::SetThreadPriority(m_handle, priority);
                DD_ASSERT(result0 == true);

                /* Set thread name */
                ::strncpy(m_name, name, ukern::MaxFiberNameLength);
            }

            virtual void StartThread() {
                s32 result0 = ::ResumeThread(m_handle);
                DD_ASSERT(result0 != -1);
            }
            virtual void WaitForThreadExit() {
                s32 result0 = ::WaitForSingleObject(m_handle, INFINITE);
                DD_ASSERT(result0 == WAIT_OBJECT_0);
            }
            virtual void ResumeThread() {
                s32 result0 = ::ResumeThread(m_handle);
                DD_ASSERT(result0 != -1);
            }
            virtual void SuspendThread() {
                s32 result0 = ::SuspendThread(m_handle);
                DD_ASSERT(result0 != -1);
            }

            virtual void SetPriority(s32 priority) {
                bool result0 = ::SetThreadPriority(m_handle, priority);
                DD_ASSERT(result0 == true);
                m_priority = priority;
            }
            virtual void SetCoreMask(u64 core_mask) {
                bool result0 = ::SetThreadAffinityMask(m_handle, core_mask);
                DD_ASSERT(result0 == true);
                m_core_mask = core_mask;
            }
	};
}
