#pragma once

namespace dd::ukern {
    
    class InternalCriticalSection {
        private:
            friend class InternalConditionVariable;
        private:
            UKernHandle m_handle;
        public:
            constexpr ALWAYS_INLINE InternalCriticalSection() : m_handle(0) {/*...*/}
            
            void Enter() {

                const FiberLocalStorage *fiber_local = ukern::GetCurrentThread();
                const UKernHandle tag = fiber_local->ukern_fiber_handle;

                while (true) {
                    
                    /* Try to acquire the critical section */
                    const UKernHandle other_waiter = ::InterlockedCompareExchangeAcquire(std::addressof(m_handle), tag, 0);
                    if (other_waiter == 0) { return; }

                    /* Set tag bit */
                    ::InterlockedBitTestAndSet(reinterpret_cast<volatile long int*>(std::addressof(m_handle)), 0x1e);

                    /* If we fail, arbitrate lock */
                    impl::GetScheduler()->ArbitrateLockImpl(other_waiter, std::addressof(m_handle), tag);
                }
            }
            
            void Leave() {

                /* Clear the address */
                const UKernHandle tag = ::InterlockedExchange(std::addressof(m_handle), 0);

                /* Unlock waiters */
                if (((tag >> 0x1e) & 1) == 1) { impl::GetScheduler()->ArbitrateUnlockImpl(std::addressof(m_handle)); }
            }

            void lock();
            void unlock();
            void try_lock();
            
            bool IsLockedByCurrentThread();
    };
    static_assert(sizeof(InternalCriticalSection) == sizeof(UKernHandle));
}
