#pragma once

namespace dd::ukern::impl {

    TickSpan GetAbsoluteTimeToWakeup(TimeSpan timeout_ns);

    class UserScheduler {
        public:
            friend class ScopedSchedulerLock;
            friend class WaitableObject;
            friend class LockArbiter;
            friend class KeyArbiter;
            friend class WaitAddressArbiter;
        private:
            using HighPriorityList        = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
            using AboveNormalPriorityList = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
            using NormalPriorityList      = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
            using BelowNormalPriorityList = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
            using LowPriorityList         = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
            using SuspendList             = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
            using WaitList                = util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::scheduler_list_node>::List;
        protected:
            SRWLOCK                   m_scheduler_lock;
            HANDLE                    m_scheduler_thread_table[MaxCoreCount];
            void                     *m_scheduler_fiber_table[MaxCoreCount];
            HighPriorityList          m_high_priority_list;
            AboveNormalPriorityList   m_above_normal_priority_list;
            NormalPriorityList        m_normal_priority_list;
            BelowNormalPriorityList   m_below_normal_priority_list;
            LowPriorityList           m_low_priority_list;
            SRWLOCK                   m_suspend_lock;
            SuspendList               m_suspended_list;
            WaitList                  m_wait_list;
            u64                       m_next_wakeup_time;
            u32                       m_current_fiber_fls_slot;
            u32                       m_allocated_user_threads;
            UKernCoreMask             m_core_mask;
            u32                       m_core_count;
            u32                       m_active_cores;
            u32                       m_runnable_fibers;
            HandleTable               m_handle_table;
        private:
            static long unsigned int InternalSchedulerThreadMain(void *arg) {

                /* Assume argument is the scheduler thread core number */
                const size_t core_number = reinterpret_cast<size_t>(arg);

                UserScheduler *scheduler = impl::GetScheduler();

                /* Convert thread to Fiber */
                scheduler->m_scheduler_fiber_table[core_number] = ::ConvertThreadToFiber(nullptr);

                /* Call into the scheduler */
                scheduler->SchedulerFiberMain(core_number);

                return 0;
            }

            static void InternalSchedulerMainThreadFiberMain([[maybe_unused]] void *arg) {

                /* Call into the scheduler */
                UserScheduler *scheduler = impl::GetScheduler();
                scheduler->SchedulerFiberMain(0);
            }

            void SchedulerFiberMain(size_t core_number);

            static void UserFiberMain(void *arg) {
                FiberLocalStorage *fiber_local = reinterpret_cast<FiberLocalStorage*>(arg);
                UserScheduler *scheduler = impl::GetScheduler();

                /* Add fiber local to fls */
                ::FlsSetValue(scheduler->m_current_fiber_fls_slot, fiber_local);

                /* Set fiber core number */
                fiber_local->current_core = ::GetCurrentProcessorNumber();

                /* Dispatch user fiber */
                (fiber_local->user_function)(fiber_local->user_arg);

                /* Exit */
                scheduler->ExitThreadImpl();
            }
        private:
            void AddToSchedulerUnsafe(FiberLocalStorage *fiber_local) {
                
                /* Handle suspension */
                if (fiber_local->activity_level == ActivityLevel_Suspended) {
                    fiber_local->fiber_state = FiberState_Suspended;
                    m_suspended_list.PushBack(*fiber_local);
                    return;
                }
                
                /* Insert into runnable list based on priority */
                if (fiber_local->priority == (THREAD_PRIORITY_NORMAL + WindowsToUKernPriorityOffset)) {
                    m_normal_priority_list.PushBack(*fiber_local);
                } else if (fiber_local->priority == (THREAD_PRIORITY_ABOVE_NORMAL + WindowsToUKernPriorityOffset)) {
                    m_above_normal_priority_list.PushBack(*fiber_local);
                } else if (fiber_local->priority == (THREAD_PRIORITY_BELOW_NORMAL + WindowsToUKernPriorityOffset)) {
                    m_below_normal_priority_list.PushBack(*fiber_local);
                } else if (fiber_local->priority == (THREAD_PRIORITY_HIGHEST + WindowsToUKernPriorityOffset)) {
                    m_high_priority_list.PushBack(*fiber_local);
                } else if (fiber_local->priority == (THREAD_PRIORITY_LOWEST + WindowsToUKernPriorityOffset)) {
                    m_low_priority_list.PushBack(*fiber_local);
                }
                
                ++m_runnable_fibers;
                
                /* Wake a sleeping core */
                ::WakeByAddressSingle(std::addressof(m_runnable_fibers));
            }
            
            void Dispatch(FiberLocalStorage *fiber_local, u32 core_number);
        public:
            constexpr ALWAYS_INLINE UserScheduler() {/*...*/}
            
            void Initialize(UKernCoreMask core_mask);
        private:
            void SwapLockForSignalKey(FiberLocalStorage *waiting_fiber);
        public:
            Result CreateThreadImpl(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, s32 priority, u32 core_id);
            
            Result StartThread(UKernHandle handle);
            void   ExitThreadImpl();
            
            Result SetPriorityImpl(UKernHandle handle, u32 priority);
            Result SetCoreMaskImpl(UKernHandle handle, u64 new_mask);
            Result SetActivityImpl(UKernHandle handle, u16 activity_level);
            
            void   SleepThreadImpl(s64 absolute_timeout);

            Result ArbitrateLockImpl(UKernHandle handle, u32 *address, u32 tag);
            Result ArbitrateUnlockImpl(u32 *address);

            Result WaitKeyImpl(u32 *address, u32 *cv_key, u32 tag, s64 absolute_timeout);
            Result SignalKeyImpl(u32 *cv_key, u32 count);

            Result WaitForAddressIfEqualImpl(u32 *address, u32 value, s64 absolute_timeout);
            Result WaitForAddressIfLessThanImpl(u32 *address, u32 value, s64 absolute_timeout, bool do_decrement);
            Result WakeByAddressImpl(u32 *address, u32 count);
            Result WakeByAddressIncrementEqualImpl(u32 *address, u32 value, u32 count);
            Result WakeByAddressModifyLessThanImpl(u32 *address, u32 value, u32 count);

            ALWAYS_INLINE FiberLocalStorage *GetCurrentThreadImpl() {
                return reinterpret_cast<FiberLocalStorage*>(::FlsGetValue(m_current_fiber_fls_slot));
            }

            static void SetInitialFiberNameUnsafe(FiberLocalStorage *fiber_local) {
                /* Special case for main thread */
                if (fiber_local->ukern_fiber_handle == 1) { ::strncpy(fiber_local->fiber_name_storage, "MainThread", MaxFiberNameLength); return; }

                /* Otherwise use fiber's initial function address */
                ::snprintf(fiber_local->fiber_name_storage, MaxFiberNameLength, "Thread0x%08x", reinterpret_cast<size_t>(fiber_local->user_function));
            }

            constexpr ALWAYS_INLINE void *GetSchedulerFiber(FiberLocalStorage *fiber_local) {
                return m_scheduler_fiber_table[fiber_local->current_core];
            }
    
            FiberLocalStorage *GetFiberByHandle(UKernHandle handle);
        public:
            void SuspendAllOtherCoresImpl() {
                
                /* Get current core number */
                FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();
                const u32 current_core = current_fiber->current_core;
    
                /* Suspend all cores except current */
                for (u32 i = 0; i < m_core_count; ++i) {
                    if (current_core != i) {
                        ::SuspendThread(m_scheduler_thread_table[i]);
                    }
                }
            }
            
            void OutputBackTraceImpl(HANDLE file) {

                /* Print backtrace for this fiber */
                FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

                /* Iterate through all other fibers for backtrace */
                ::puts("Backtrace was called. But it's not yet implemented.");
                
            }
    };
    
    class ScopedSchedulerLock {
        private:
            UserScheduler *m_scheduler;
        public:
            explicit ALWAYS_INLINE ScopedSchedulerLock(UserScheduler *scheduler) : m_scheduler(scheduler) {
                ::AcquireSRWLockExclusive(std::addressof(m_scheduler->m_scheduler_lock));
            }

            ALWAYS_INLINE ~ScopedSchedulerLock() {
                ::ReleaseSRWLockExclusive(std::addressof(m_scheduler->m_scheduler_lock));
            }
    };
}
