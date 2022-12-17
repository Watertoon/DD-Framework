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
#include <dd.hpp>

namespace dd::ukern::impl {

    constinit util::FixedObjectAllocator<FiberLocalStorage, MaxThreadCount> UserFiberLocalAllocator = {};

    TickSpan GetAbsoluteTimeToWakeup(TimeSpan timeout_ns) {

        /* Convert to tick */
        const s64 timeout_tick = timeout_ns.GetTick();

        /* "Infinite" time on zero */
        if (0 >= timeout_tick) { return TimeSpan::MaxTime; }

        /* Calculate absolute timeout */
        const s64 absolute_time = timeout_tick + util::GetSystemTick();

        if (absolute_time + 1 <= 1) { return TimeSpan::MaxTime; }

        return absolute_time + 2;
    }

    NO_RETURN void UserScheduler::SchedulerFiberMain(size_t core_num) {

        /* Label for post dispatch/rest restart */
        _ukern_scheduler_restart:
        
        const u32 core_number = core_num;

        /* Get current time for fibers on a timeout */
        const u64 tick = util::GetSystemTick();

        /* Visit waiting thread list for timeouts */
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {

            /* Check if the waiter has become schedulable */
            if (waiting_fiber.IsSchedulable(core_number, tick) == false) { continue; }

            /* Cancel the wait */
            waiting_fiber.waitable_object->CancelWait(std::addressof(waiting_fiber), ResultTimeout);
        }

        /* Run first available thread by priority */
        for (FiberLocalStorage &runnable_fiber : m_high_priority_list) {
            if (runnable_fiber.IsSchedulable(core_number, tick) == true) { this->Dispatch(std::addressof(runnable_fiber), core_number); goto _ukern_scheduler_restart; }
        }
        for (FiberLocalStorage &runnable_fiber : m_above_normal_priority_list) {
            if (runnable_fiber.IsSchedulable(core_number, tick) == true) { this->Dispatch(std::addressof(runnable_fiber), core_number); goto _ukern_scheduler_restart; }
        }
        for (FiberLocalStorage &runnable_fiber : m_normal_priority_list) {
            if (runnable_fiber.IsSchedulable(core_number, tick) == true) { this->Dispatch(std::addressof(runnable_fiber), core_number); goto _ukern_scheduler_restart; }
        }
        for (FiberLocalStorage &runnable_fiber : m_below_normal_priority_list) {
            if (runnable_fiber.IsSchedulable(core_number, tick) == true) { this->Dispatch(std::addressof(runnable_fiber), core_number); goto _ukern_scheduler_restart; }
        }
        for (FiberLocalStorage &runnable_fiber : m_low_priority_list) {
            if (runnable_fiber.IsSchedulable(core_number, tick) == true) { this->Dispatch(std::addressof(runnable_fiber), core_number); goto _ukern_scheduler_restart; }
        }

        --m_active_cores;

        /* If we are the last core, and there are no runnable fibers, then we have deadlocked */
        if (m_runnable_fibers == 0 && m_active_cores == 0) { 
            /* Kill process */
            DD_ASSERT(false);
        } else if (m_active_cores == 0) {
            /* Alert another waiting core to check if they can run a fiber */
            ::WakeByAddressSingle(std::addressof(m_runnable_fibers));
        }

        do {
            /* Find next wakeup time */
            u64 timeout_tick = 0xffff'ffff'ffff'ffff;
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if ((waiting_fiber.core_mask & (1 << core_number)) != 0  && waiting_fiber.timeout < timeout_tick)  { timeout_tick = waiting_fiber.timeout; }
            }
            for (FiberLocalStorage &runnable_fiber : m_high_priority_list) {
                if ((runnable_fiber.core_mask & (1 << core_number)) != 0 && runnable_fiber.timeout < timeout_tick) { timeout_tick = runnable_fiber.timeout; }
            }
            for (FiberLocalStorage &runnable_fiber : m_above_normal_priority_list) {
                if ((runnable_fiber.core_mask & (1 << core_number)) != 0 && runnable_fiber.timeout < timeout_tick) { timeout_tick = runnable_fiber.timeout; }
            }
            for (FiberLocalStorage &runnable_fiber : m_normal_priority_list) {
                if ((runnable_fiber.core_mask & (1 << core_number)) != 0 && runnable_fiber.timeout < timeout_tick) { timeout_tick = runnable_fiber.timeout; }
            }
            for (FiberLocalStorage &runnable_fiber : m_below_normal_priority_list) {
                if ((runnable_fiber.core_mask & (1 << core_number)) != 0 && runnable_fiber.timeout < timeout_tick) { timeout_tick = runnable_fiber.timeout; }
            }
            for (FiberLocalStorage &runnable_fiber : m_low_priority_list) {
                if ((runnable_fiber.core_mask & (1 << core_number)) != 0 && runnable_fiber.timeout < timeout_tick) { timeout_tick = runnable_fiber.timeout; }
            }

            /* Release scheduler lock */
            ::ReleaseSRWLockExclusive(std::addressof(m_scheduler_lock));

            /* Rest core until a new fiber is schedulable, or another rester is looking to sleep */
            u32 wait_value = m_runnable_fibers;
            u32 time_left = dd::TimeSpan::GetTimeLeftOnTarget(timeout_tick).GetMilliSeconds();
            if (time_left == 0) { ::AcquireSRWLockExclusive(std::addressof(m_scheduler_lock)); break; }

            ::WaitOnAddress(std::addressof(m_runnable_fibers), std::addressof(wait_value), sizeof(u32), time_left);

            /* Reacquire scheduler lock */
            ::AcquireSRWLockExclusive(std::addressof(m_scheduler_lock));
        } while (m_runnable_fibers == 0);

        ++m_active_cores;

        /* Attempt to schedule a fiber */
        goto _ukern_scheduler_restart;
    }

    void UserScheduler::Dispatch(FiberLocalStorage *fiber_local, u32 core_number) {

        /* Set fiber runtime args */
        fiber_local->fiber_state = FiberState_Running;
        fiber_local->current_core = core_number;
        DD_ASSERT(m_core_count > fiber_local->current_core);
        
        /* Delist from scheduler */
        fiber_local->scheduler_list_node.Unlink();

        /* Switch to user fiber */
        ::SwitchToFiber(fiber_local->win32_fiber_handle);

        DD_ASSERT(core_number == fiber_local->current_core);

        /* Handle previous fiber */
        switch (fiber_local->fiber_state) {
            case FiberState_Running:
                /* Readd the fiber to the scheduler */
                this->AddToSchedulerUnsafe(fiber_local);

                break;
            case FiberState_Exiting:
                /* Delete Win32 fiber */
                ::DeleteFiber(fiber_local->win32_fiber_handle);

                /* Free fiber local */
                UserFiberLocalAllocator.Free(fiber_local);

                break;
            case FiberState_Waiting:
                break;
            default:
                DD_ASSERT(false);
        }

        return;
    }

	void UserScheduler::Initialize(UKernCoreMask core_mask) {

		/* Get and set initial core count */
		const u32 core_count = util::CountOneBits64(core_mask);
		m_active_cores = core_count;
		m_core_count   = core_count;
        m_core_mask    = core_mask;

		/* Set main thread core mask to core 0 */
		const u64 main_thread_mask = 1;
		::SetThreadAffinityMask(::GetCurrentThread(), main_thread_mask);
        
        /* Initialize handle table */
        m_handle_table.Initialize();

		/* Set main thread handle */
		HANDLE main_thread_handle = nullptr;
		const bool result0 = ::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess(), std::addressof(main_thread_handle), 0, false, DUPLICATE_SAME_ACCESS);
        DD_ASSERT(result0 == true);
		m_scheduler_thread_table[0] = main_thread_handle;

		/* Setup main thread fiber local */
		FiberLocalStorage *main_fiber_local = UserFiberLocalAllocator.Allocate();

		main_fiber_local->priority           = 2;
		main_fiber_local->current_core       = 0;
		main_fiber_local->core_mask          = 1;
		main_fiber_local->fiber_state        = FiberState_Running;
		main_fiber_local->activity_level     = ActivityLevel_Schedulable;
        main_fiber_local->win32_fiber_handle = ::ConvertThreadToFiber(main_fiber_local);
        DD_ASSERT(main_fiber_local->win32_fiber_handle != nullptr);

        /* Create main thread scheduler fiber */
		m_scheduler_fiber_table[0] = ::CreateFiber(0x2000, InternalSchedulerMainThreadFiberMain, main_fiber_local);
        DD_ASSERT(m_scheduler_fiber_table[0] != nullptr);

        /* Reserve main thread */
		const bool result1 = m_handle_table.ReserveHandle(std::addressof(main_fiber_local->ukern_fiber_handle), main_fiber_local);
        DD_ASSERT(result1 == true);

		this->SetInitialFiberNameUnsafe(main_fiber_local);

		/* Allocate scheduler worker fibers */
		for (u32 i = 1; i < core_count; ++i) {
			m_scheduler_thread_table[i] = ::CreateThread(nullptr, 0x1000, InternalSchedulerFiberMain, reinterpret_cast<void*>(i), CREATE_SUSPENDED, nullptr);
			DD_ASSERT(m_scheduler_thread_table[i] != INVALID_HANDLE_VALUE);

            u64 secondary_mask = (1 << i);
			::SetThreadAffinityMask(m_scheduler_thread_table[i], secondary_mask);
			::ResumeThread(m_scheduler_thread_table[i]);
		}

		return;
	}

    FiberLocalStorage *UserScheduler::GetFiberByHandle(UKernHandle handle) {
        return reinterpret_cast<FiberLocalStorage*>(m_handle_table.GetObjectByHandle(handle));
    }

    /* Service api */
    Result UserScheduler::CreateThreadImpl(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, s32 priority, u32 core_id) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(thread_func != nullptr,              ResultInvalidThreadFunctionPointer);
        RESULT_RETURN_UNLESS(stack_size  != 0,                    ResultInvalidStackSize);
        RESULT_RETURN_UNLESS(-2 <= priority && priority <= 2,     ResultInvalidPriority);
        RESULT_RETURN_UNLESS(((1 << core_id) & m_core_mask) != 0, ResultInvalidCoreId);

        /* Lock the scheduler */
        ScopedSchedulerLock lock(this);

        /* Try to acquire a freed fiber slot from the free list */
        FiberLocalStorage *fiber_local = UserFiberLocalAllocator.Allocate();
        RESULT_RETURN_IF(fiber_local == nullptr, ResultThreadStorageExhaustion);

        /* Try to reserve a ukern handle */
        const bool result = m_handle_table.ReserveHandle(std::addressof(fiber_local->ukern_fiber_handle), fiber_local);
        RESULT_RETURN_IF(result == false, ResultHandleExhaustion);

        /* Set fiber args */
        fiber_local->priority       = priority + WindowsToUKernPriorityOffset;
        fiber_local->stack_size     = stack_size;
        fiber_local->core_mask      = (1 << core_id);
        fiber_local->user_arg       = reinterpret_cast<void*>(arg);
        fiber_local->user_function  = thread_func;
        fiber_local->fiber_state    = FiberState_Suspended;
        fiber_local->activity_level = ActivityLevel_Suspended;

        this->SetInitialFiberNameUnsafe(fiber_local);

        /* Create win32 fiber */
        fiber_local->win32_fiber_handle = ::CreateFiber(stack_size, UserFiberMain, fiber_local);
        DD_ASSERT(fiber_local->win32_fiber_handle != nullptr);

        /* Add to suspend list */
        m_suspended_list.PushBack(*fiber_local);

        *out_handle = fiber_local->ukern_fiber_handle;

        return ResultSuccess;
    }

    void UserScheduler::ExitFiberImpl() {

        /* Get current fiber */
        FiberLocalStorage *fiber_local = this->GetCurrentThreadImpl();

        /* Acquire scheduler lock */
        ::AcquireSRWLockExclusive(std::addressof(m_scheduler_lock));

        /* Set state */
        fiber_local->fiber_state = FiberState_Exiting;

        /* Swap to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(fiber_local));
    }

    void UserScheduler::ExitThreadImpl(UKernHandle handle) {
        FiberLocalStorage *exit_fiber = this->GetFiberByHandle(handle);

        /* Sleep loop while handle is valid */
        while (exit_fiber != nullptr) {
            this->SleepThreadImpl(0);
            exit_fiber = this->GetFiberByHandle(handle);
        }
    }

    Result UserScheduler::SetPriorityImpl(UKernHandle handle, s32 priority) {
    
        /* Verify input */
        if (-2 <= priority && priority <= 2) { return ResultInvalidPriority; }
        
        /* Get fiber by handle  */
        FiberLocalStorage *fiber_local = this->GetFiberByHandle(handle);
        RESULT_RETURN_UNLESS(fiber_local != nullptr, ResultInvalidHandle)

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);
        
        /* Same value check */
        if (fiber_local->priority == priority) { return ResultSamePriority; }

        /* Change priority */
        fiber_local->priority = priority;

        /* Reschedule if necessary */
        if (fiber_local->fiber_state == FiberState_Scheduled) {
            fiber_local->scheduler_list_node.Unlink();
            this->AddToSchedulerUnsafe(fiber_local);
        }

        return ResultSuccess;
    }

    Result UserScheduler::SetCoreMaskImpl(UKernHandle handle, UKernCoreMask core_mask) {
        
        /* Get fiber by handle  */
        FiberLocalStorage *fiber_local = this->GetFiberByHandle(handle);
        RESULT_RETURN_UNLESS(fiber_local != nullptr, ResultInvalidHandle);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);
        
        /* Same value check */
        if (fiber_local->core_mask == core_mask) { return ResultSameCoreMask; }
        
        /* Change core mask */
        fiber_local->core_mask = core_mask;

        /* Reschedule if necessary */
        if (fiber_local->fiber_state == FiberState_Scheduled) {
            fiber_local->scheduler_list_node.Unlink();
            this->AddToSchedulerUnsafe(fiber_local);
        }
        
        return ResultSuccess;
    }

    Result UserScheduler::SetActivityImpl(UKernHandle handle, u16 activity_level) {

        /* Get fiber by handle  */
        FiberLocalStorage *fiber_local = this->GetFiberByHandle(handle);

        /* Integrity check */
        RESULT_RETURN_IF(fiber_local == nullptr, ResultInvalidHandle);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);
        
        /* Same value check */
        RESULT_RETURN_IF(fiber_local->activity_level == activity_level, ResultSameActivityLevel);

        fiber_local->activity_level = static_cast<ActivityLevel>(activity_level);

        /* Reschedule if necessary */
        if (fiber_local->fiber_state == FiberState_Scheduled || fiber_local->fiber_state == FiberState_Suspended) {
            fiber_local->scheduler_list_node.Unlink();
            this->AddToSchedulerUnsafe(fiber_local);
        }

        return ResultSuccess;
    }

    void UserScheduler::SleepThreadImpl(u64 absolute_timeout) {

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        ScopedSchedulerLock lock(this);

        /* Only set timeout if required */
        if (absolute_timeout != 0) {
            /* Set timeout */
            current_fiber->timeout = absolute_timeout;
        }

        /* Switch to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        return;
    }

    Result UserScheduler::ArbitrateLockImpl(UKernHandle handle, u32 *lock_address, u32 tag) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(lock_address != nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Get fiber from handle table */
        FiberLocalStorage *handle_fiber = this->GetFiberByHandle(handle);
        RESULT_RETURN_IF(handle_fiber == nullptr || handle_fiber == current_fiber, ResultInvalidHandle);

        /* Load value from address */
        const u32 address_tag = *lock_address;
        RESULT_RETURN_IF(address_tag != (handle | FiberLocalStorage::HasChildWaitersBit), ResultInvalidLockAddressValue);

        /* Set lock state */
        LockArbiter lock_arbiter = {};
        current_fiber->waitable_object = std::addressof(lock_arbiter);
        current_fiber->lock_address    = lock_address;
        current_fiber->wait_tag        = tag;
        current_fiber->fiber_state     = FiberState_Waiting;

        /* Push back thread waiter */
        handle_fiber->wait_list.PushBack(*current_fiber);

        /* Swap fiber to suspend list */
        current_fiber->scheduler_list_node.Unlink();
        m_suspended_list.PushBack(*current_fiber);

        /* Swap to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        return current_fiber->last_result;
    }

    Result UserScheduler::ArbitrateUnlockImpl(u32 *lock_address) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(lock_address != nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Integrity checks */
        RESULT_RETURN_UNLESS(current_fiber->wait_list.IsEmpty() == false,                                                       ResultRequiresLock);
        RESULT_RETURN_UNLESS(current_fiber->ukern_fiber_handle == ((*lock_address) & (~FiberLocalStorage::HasChildWaitersBit)), ResultInvalidLockAddressValue);

        /* Release lock */
        current_fiber->ReleaseLockWaitListUnsafe();

        return ResultSuccess;
    }

    Result UserScheduler::WaitKeyImpl(u32 *lock_address, u32 *cv_key, u32 tag, s64 absolute_timeout) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(lock_address != nullptr, ResultInvalidAddress);
        RESULT_RETURN_UNLESS(cv_key != nullptr,  ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Integrity checks */
        RESULT_RETURN_UNLESS(current_fiber->wait_list.IsEmpty() == false,   ResultRequiresLock);
        RESULT_RETURN_UNLESS(current_fiber->ukern_fiber_handle == *lock_address, ResultInvalidLockAddressValue);

        /* Release lock */
        current_fiber->ReleaseLockWaitListUnsafe();

        /* Set cv key to 1 */
        *cv_key = 1;

        /* Check if timed out */
        u32 result = ResultSuccess;
        if (0 == absolute_timeout) {
            result = ResultTimeout;
        } else {
            /* Set wait state */
            KeyArbiter key_arbiter         = {};
            current_fiber->waitable_object = std::addressof(key_arbiter);
            current_fiber->wait_address    = cv_key;
            current_fiber->lock_address    = lock_address;
            current_fiber->wait_tag        = tag;
            current_fiber->fiber_state     = FiberState_Waiting;
            current_fiber->timeout         = absolute_timeout;
        }

        /* Find a parent cv waiter */
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (waiting_fiber.wait_address == cv_key) {
                waiting_fiber.wait_list.PushBack(*current_fiber);
                break;
            }
        }

        /* If no parent, become the parent */
        if (current_fiber->wait_list_node.IsLinked() == true) {
            current_fiber->scheduler_list_node.Unlink();
            m_wait_list.PushBack(*current_fiber);
        }

        /* Swap to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        if (result == ResultSuccess) {
            result = current_fiber->last_result;
        }

        return result;
    }

    void UserScheduler::SwapLockForSignalKey(FiberLocalStorage *waiting_fiber) {

        /* Try to take the lock back */
        const u32 prev_tag = *waiting_fiber->lock_address;
        u32       tag      =  waiting_fiber->wait_tag;
        if (prev_tag != 0) {
            tag |= FiberLocalStorage::HasChildWaitersBit;
        }
        *waiting_fiber->lock_address = tag;

        /* If there were other waiters */
        if (prev_tag != 0) {
            /* Get fiber by handle */
            FiberLocalStorage *lock_fiber = this->GetFiberByHandle(~FiberLocalStorage::HasChildWaitersBit);

            /* Push back fiber waiter */
            lock_fiber->wait_list.PushBack(*waiting_fiber);
        } else {
            waiting_fiber->waitable_object->EndWait(waiting_fiber, ResultSuccess);
        }
    }

    Result UserScheduler::SignalKeyImpl(u32 *cv_key, u32 signal_count) {

        /* Integrity checks */
        RESULT_RETURN_IF(cv_key == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Zero signal count */
        if (signal_count == 0) {
            /* Set cv key to 0 */
            *cv_key = 0;

            return ResultSuccess;
        }

        /* Find parent waiter */
        FiberLocalStorage *cv_fiber = nullptr;
        for (FiberLocalStorage &fiber_local : m_wait_list) {
            if (fiber_local.wait_address == cv_key) {
                cv_fiber = std::addressof(fiber_local);
                break;
            }
        }

        /* Check we found a waiter */
        RESULT_RETURN_IF(cv_fiber == nullptr, ResultInvalidAddress);

        /* Handle reacquisition of lock */
        this->SwapLockForSignalKey(cv_fiber);

        /* Unlock cv waiters */
        u32 i = 1;
        for (FiberLocalStorage &waiting_fiber : cv_fiber->wait_list) {

            /* Break if signal count is reached */
            if (signal_count <= i) { break; }

            /* Detach from child list */
            waiting_fiber.wait_list_node.Unlink();

            /* Handle reacquisition of lock */
            this->SwapLockForSignalKey(std::addressof(waiting_fiber));

            /* Loop count */
            ++i;
        }

        /* Transfer wait list */
        if (cv_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_cv_parent = std::addressof(cv_fiber->wait_list.PopFront());
            for (FiberLocalStorage &waiting_fiber : cv_fiber->wait_list) {
                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_cv_parent->wait_list.PushBack(waiting_fiber);
            }

            cv_fiber->scheduler_list_node.Unlink();
            m_wait_list.PushBack(*next_cv_parent);
        }

        /* Set cv key to 0 */
        *cv_key = 0;

        return ResultSuccess;
    }

    Result UserScheduler::WaitForAddressIfEqualImpl(u32 *wait_address, u32 value, s64 absolute_timeout) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Check address */
        u32 result = ResultSuccess;
        if (*wait_address != value) {
            result = ResultInvalidWaitAddressValue;
        } else if (absolute_timeout <= 0) {
            result = ResultTimeout;
        } else {

            /* Set wait address state */
            WaitAddressArbiter wait_address_arbiter = {};
            current_fiber->waitable_object  = std::addressof(wait_address_arbiter);
            current_fiber->wait_address = wait_address;
            current_fiber->fiber_state  = FiberState_Waiting;
            current_fiber->timeout      = absolute_timeout;

            /* Find address in wait list */
            FiberLocalStorage *address_fiber = nullptr;
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if (wait_address == waiting_fiber.wait_address) {
                    address_fiber = std::addressof(waiting_fiber);
                    break;
                }
            }

            /* Push back to a wait list */
            if (address_fiber == nullptr) {
                m_wait_list.PushBack(*current_fiber);
            } else {
                address_fiber->wait_list.PushBack(*current_fiber);
            }
        }

        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        if (result == ResultSuccess) {
            result = current_fiber->last_result;
        }

        return result;
    }

    Result UserScheduler::WaitForAddressIfLessThanImpl(u32 *wait_address, u32 value, s64 absolute_timeout, bool do_decrement) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Perform decrement */
        u32 wait_value = *wait_address;
        if (do_decrement == true) {
            *wait_address  = *wait_address - 1;
        }

        /* Check address */
        u32 result = ResultSuccess;
        if (wait_value >= value) {
            result = ResultInvalidWaitAddressValue;
        } else if (absolute_timeout <= 0) {
            result = ResultTimeout;
        } else {

            /* Set wait address state */
            WaitAddressArbiter wait_address_arbiter = {};
            current_fiber->waitable_object  = std::addressof(wait_address_arbiter);
            current_fiber->wait_address = wait_address;
            current_fiber->fiber_state  = FiberState_Waiting;
            current_fiber->timeout      = absolute_timeout;

            /* Find address in wait list */
            FiberLocalStorage *address_fiber = nullptr;
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if (wait_address == waiting_fiber.wait_address) {
                    address_fiber = std::addressof(waiting_fiber);
                    break;
                }
            }

            /* Push back to a wait list */
            if (address_fiber == nullptr) {
                m_wait_list.PushBack(*current_fiber);
            } else {
                address_fiber->wait_list.PushBack(*current_fiber);
            }
        }

        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        if (result == ResultSuccess) {
            result = current_fiber->last_result;
        }

        return result;
    }

    Result UserScheduler::WakeByAddressImpl(u32 *wait_address, u32 count) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        if (count == 0) { return ResultSuccess; }

        /* Find address in wait list */
        FiberLocalStorage *address_fiber = nullptr;
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (wait_address == waiting_fiber.wait_address) {
                address_fiber = std::addressof(waiting_fiber);
                break;
            }
        }

        /* Release parent waiter */
        address_fiber->waitable_object->EndWait(address_fiber, ResultSuccess);

        /* Release the waiting fibers */
        u32 i = 1;
        for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {

            if (count < i) { break; }

            waiting_fiber.waitable_object->EndWait(std::addressof(waiting_fiber), ResultSuccess);
            waiting_fiber.wait_list_node.Unlink();

            ++i;
        }

        /* Transfer wait list */
        if (address_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_address_parent = std::addressof(address_fiber->wait_list.PopFront());
            for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {
                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_address_parent->wait_list.PushBack(waiting_fiber);
            }

            address_fiber->scheduler_list_node.Unlink();
            m_wait_list.PushBack(*next_address_parent);
        }

       return ResultSuccess;
    }

    Result UserScheduler::WakeByAddressIncrementEqualImpl(u32 *wait_address, u32 value, u32 count) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);
        
        /* Check address value */
        RESULT_RETURN_IF(*wait_address != value, ResultValueOutOfRange);

        *wait_address = value + 1;

        /* Find address in wait list */
        FiberLocalStorage *address_fiber = nullptr;
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (wait_address == waiting_fiber.wait_address) {
                address_fiber = std::addressof(waiting_fiber);
                break;
            }
        }
        
        /* Release parent waiter */
        address_fiber->waitable_object->EndWait(address_fiber, ResultSuccess);

        /* Release the waiting fibers */
        u32 i = 1;
        for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {

            if (count < i) { break; }

            waiting_fiber.waitable_object->EndWait(std::addressof(waiting_fiber), ResultSuccess);
            waiting_fiber.wait_list_node.Unlink();

            ++i;
        }

        /* Transfer wait list */
        if (address_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_address_parent = std::addressof(address_fiber->wait_list.PopFront());
            for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {
                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_address_parent->wait_list.PushBack(waiting_fiber);
            }

            address_fiber->scheduler_list_node.Unlink();
            m_wait_list.PushBack(*next_address_parent);
        }

        return ResultSuccess;
    }

    Result UserScheduler::WakeByAddressModifyLessThanImpl(u32 *wait_address, u32 value, u32 count) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Check address value */
        if (*wait_address != value) {
            return ResultValueOutOfRange;
        }

        /* Find address in wait list */
        FiberLocalStorage *address_fiber = nullptr;
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (wait_address == waiting_fiber.wait_address) {
                address_fiber = std::addressof(waiting_fiber);
                break;
            }
        }

        /* Determine value to signal */
        u32 signal = -1;

        if (address_fiber == nullptr) { 
            signal = 1; 
        } else if (0 < count) {
            /* Count waiters */
            signal = 0;
            u32 i = 1;
            for ([[maybe_unused]] FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {
                ++i;
            }
            /* Signal -1 if less waiters to wakeup then count */
            if (i < count) { signal = -1; }
        }

        /* Only modify if non-zero signal */
        if (signal != 0) {
            *wait_address = value + signal;
        }

        /* Release parent waiter */
        address_fiber->waitable_object->EndWait(address_fiber, ResultSuccess);

        /* Release the waiting fibers */
        u32 i = 1;
        for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {

            if (count < i) { break; }

            waiting_fiber.waitable_object->EndWait(std::addressof(waiting_fiber), ResultSuccess);
            waiting_fiber.wait_list_node.Unlink();

            ++i;
        }

        /* Transfer wait list */
        if (address_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_address_parent = std::addressof(address_fiber->wait_list.PopFront());
            for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {
                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_address_parent->wait_list.PushBack(waiting_fiber);
            }

            address_fiber->scheduler_list_node.Unlink();
            m_wait_list.PushBack(*next_address_parent);
        }

        return ResultSuccess;
    }

}
