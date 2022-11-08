 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
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

                /* Acquire loop */
                for (;;) {
                    /* Try to acquire the critical section */
                    const UKernHandle other_waiter = ::InterlockedCompareExchangeAcquire(std::addressof(m_handle), tag, 0);
                    if (other_waiter == 0) { return; }

                    /* Set tag bit */
                    if (((other_waiter >> 0x1e) & 1) == 0) {
                        const UKernHandle prev_value = ::InterlockedOr(reinterpret_cast<volatile long int*>(std::addressof(m_handle)), FiberLocalStorage::HasChildWaitersBit);
                        if (prev_value != other_waiter) { continue; }
                    }

                    /* If we fail, lock the thread */
                    RESULT_ABORT_UNLESS(impl::GetScheduler()->ArbitrateLockImpl(other_waiter & (~FiberLocalStorage::HasChildWaitersBit), std::addressof(m_handle), tag), ResultSuccess);
                    if ((m_handle & (~FiberLocalStorage::HasChildWaitersBit)) != tag) {
                        return;
                    }
                }
            }

            void Leave() {

                /* Clear the address */
                const UKernHandle tag = ::InterlockedExchange(std::addressof(m_handle), 0);

                /* Unlock waiters */
                if (((tag >> 0x1e) & 1) == 1) { RESULT_ABORT_UNLESS(impl::GetScheduler()->ArbitrateUnlockImpl(std::addressof(m_handle)), ResultSuccess); }
            }

            void lock();
            void unlock();
            void try_lock();
            
            bool IsLockedByCurrentThread();
    };
    static_assert(sizeof(InternalCriticalSection) == sizeof(UKernHandle));
}
