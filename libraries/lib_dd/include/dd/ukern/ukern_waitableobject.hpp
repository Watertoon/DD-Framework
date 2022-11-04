#pragma once

namespace dd::ukern::impl {

    class WaitableObject {
        public:
            constexpr WaitableObject() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result);
            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result);

            void EndFiberWaitImpl(FiberLocalStorage *wait_fiber, Result wait_result) {

                /* Remove from suspend/wait list */
                wait_fiber->scheduler_list_node.Unlink();

                /* Set Fiber state */
                wait_fiber->fiber_state = FiberState_Scheduled;
                wait_fiber->last_result = wait_result;

                /* Add to scheduler */
                GetScheduler()->AddToSchedulerUnsafe(wait_fiber);
            }
    };
    
    class LockArbiter : public WaitableObject {
        public:
            constexpr LockArbiter() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                EndFiberWaitImpl(wait_fiber, wait_result);
            }

            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result) override {

                /* Remove from child list */
                wait_fiber->ReleaseLockWaitListUnsafe();

                EndFiberWaitImpl(wait_fiber, wait_result);
            }
    };

    class KeyArbiter : public WaitableObject {
        public:
            constexpr KeyArbiter() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                EndFiberWaitImpl(wait_fiber, wait_result);
            }

            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result) override {

                /* Transfer wait list or remove from child list */
                if (wait_fiber->wait_list.IsEmpty() == false) {

                    FiberLocalStorage *next_cv_parent = std::addressof(wait_fiber->wait_list.PopFront());
                    for (FiberLocalStorage &waiting_fiber : wait_fiber->wait_list) {
                        /* Detach from previous list */
                        waiting_fiber.wait_list_node.Unlink();

                        /* Add to new parent */
                        next_cv_parent->wait_list.PushBack(waiting_fiber);
                    }
                } else if (wait_fiber->wait_list_node.IsLinked() == true) {
                    wait_fiber->wait_list_node.Unlink();
                } else {
                    EndFiberWaitImpl(wait_fiber, wait_result);
                    return;
                }
                
                /* Try to take the lock back */
                const u32 prev_tag = *wait_fiber->lock_address;
                u32       tag      =  wait_fiber->wait_tag;
                if (prev_tag != 0) {
                    tag |= FiberLocalStorage::HasChildWaitersBit;
                }
                *wait_fiber->lock_address = tag;

                /* If there were other waiters */
                if (prev_tag != 0) {
                    /* Get fiber by handle */
                    FiberLocalStorage *lock_fiber = impl::GetScheduler()->GetFiberByHandle(prev_tag & ~FiberLocalStorage::HasChildWaitersBit);

                    /* Push back fiber waiter */
                    lock_fiber->wait_list.PushBack(*wait_fiber);
                    
                    /* Swap to suspend list */
                    lock_fiber->scheduler_list_node.Unlink();
                    impl::GetScheduler()->m_suspended_list.PushBack(*lock_fiber);
                    
                } else {
                    EndFiberWaitImpl(wait_fiber, wait_result);
                }

                return;
            }
    };

    class WaitAddressArbiter : public WaitableObject {
        public:
            constexpr WaitAddressArbiter() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                EndFiberWaitImpl(wait_fiber, wait_result);
            }

            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                
                EndFiberWaitImpl(wait_fiber, wait_result);
            }
    };
}