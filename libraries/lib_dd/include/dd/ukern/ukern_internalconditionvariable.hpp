#pragma once

namespace dd::ukern {
    
    class InternalConditionVariable {
        private:
            u32 m_cv;
        public:
            constexpr InternalConditionVariable() {/*...*/}
            
            void Wait(InternalCriticalSection *cs) {
                
                const FiberLocalStorage *fiber_local = ukern::GetCurrentThread();
                const UKernHandle tag = fiber_local->ukern_fiber_handle;
                
                DD_ASSERT(tag == (cs->m_handle & (~0x4000'0000)));
                
                impl::GetScheduler()->WaitKeyImpl(std::addressof(cs->m_handle), std::addressof(m_cv), tag, -1);
            }
            
            void TimedWait(InternalCriticalSection *cs, s64 timeout_ns) {
                
                const FiberLocalStorage *fiber_local = ukern::GetCurrentThread();
                const UKernHandle tag = fiber_local->ukern_fiber_handle;
                
                DD_ASSERT(tag == (cs->m_handle & (~0x4000'0000)));
                
                impl::GetScheduler()->WaitKeyImpl(std::addressof(cs->m_handle), std::addressof(m_cv), tag, impl::GetAbsoluteTimeToWakeup(timeout_ns));
            }
            
            void Signal(u32 count = 1) {
                if (m_cv != 0) {
                    impl::GetScheduler()->SignalKeyImpl(std::addressof(m_cv), count);
                }
            }
            void Broadcast() {
                if (m_cv != 0) {
                    impl::GetScheduler()->SignalKeyImpl(std::addressof(m_cv), -1);
                }
            }
    };
}
