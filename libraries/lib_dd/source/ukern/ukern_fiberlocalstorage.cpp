#include <dd.hpp>

namespace dd::ukern {

    bool FiberLocalStorage::IsSchedulable(u32 core_number, u64 time) {
        if (core_mask & (1 << core_number)) { return false; }
        if (waitable_object != nullptr && timeout < time) { waitable_object->CancelWait(this, ResultTimeout); return true; }
        if (timeout < time) { return true; }

        return false;
    }

    void FiberLocalStorage::ReleaseLockWaitListUnsafe() {

        /* Get waiter to signal */
        FiberLocalStorage next_owner = this->wait_list.PopFront();

        /* Set wait tag and clear state */
        *next_owner.lock_address = (this->wait_list.IsEmpty() == true) ? next_owner.wait_tag : next_owner.wait_tag | HasChildWaitersBit;
        next_owner.lock_address  = nullptr;
        next_owner.wait_tag      = 0;

        /* Transfer waiters to new owner */
        for (FiberLocalStorage &waiter : this->wait_list) {
            waiter.wait_list_node.Unlink();
            next_owner.wait_list.PushBack(waiter);
        }

        /* End next owner's wait */
        next_owner.waitable_object->EndWait(std::addressof(next_owner), ResultSuccess);
        next_owner.waitable_object = nullptr;
    }
}
