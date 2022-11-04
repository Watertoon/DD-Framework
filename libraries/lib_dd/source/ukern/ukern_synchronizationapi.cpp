#include <dd.hpp>

namespace dd::ukern {
    
    Result ArbitrateLock(UKernHandle handle, uintptr_t address, u32 tag) {
        /* Call scheduler impl */
        impl::UserScheduler *scheduler = impl::GetScheduler();
        return scheduler->ArbitrateLockImpl(handle, reinterpret_cast<u32*>(address), tag);
    }

    Result ArbitrateUnlock(uintptr_t address) {
        /* Call scheduler impl */
        impl::UserScheduler *scheduler = impl::GetScheduler();
        return scheduler->ArbitrateUnlockImpl(reinterpret_cast<u32*>(address));
    }
    
    Result WaitKey(uintptr_t address, uintptr_t cv_key, u32 tag, s64 timeout_ns) {
        /* Call scheduler impl */
        impl::UserScheduler *scheduler = impl::GetScheduler();
        return scheduler->WaitKeyImpl(reinterpret_cast<u32*>(address), reinterpret_cast<u32*>(cv_key), tag, impl::GetAbsoluteTimeToWakeup(timeout_ns));
    }
    Result SignalKey(uintptr_t cv_key, u32 count) {
        /* Call scheduler impl */
        impl::UserScheduler *scheduler = impl::GetScheduler();
        return scheduler->SignalKeyImpl(reinterpret_cast<u32*>(cv_key), count);
    }

    Result WaitForAddress(uintptr_t address, u32 arbitration_type, u32 value, s64 timeout_ns) {
        impl::UserScheduler *scheduler = impl::GetScheduler();
        
        const u64 absolute_timeout = impl::GetAbsoluteTimeToWakeup(timeout_ns);
        
        if (arbitration_type == ArbitrationType_WaitIfEqual) {
            return scheduler->WaitForAddressIfEqualImpl(reinterpret_cast<u32*>(address), value, absolute_timeout);
        } else if (arbitration_type == ArbitrationType_DecrementWaitIfLessThan) {
            return scheduler->WaitForAddressIfLessThanImpl(reinterpret_cast<u32*>(address), value, absolute_timeout, true);
        } else if (arbitration_type == ArbitrationType_WaitIfLessThan) {
            return scheduler->WaitForAddressIfLessThanImpl(reinterpret_cast<u32*>(address), value, absolute_timeout, false);
        }

        return ResultInvalidArbitrationType;
    }
    Result WakeByAddress(uintptr_t address, u32 signal_type, u32 value, u32 count) {
        impl::UserScheduler *scheduler = impl::GetScheduler();
        if (signal_type == SignalType_Signal) {
            return scheduler->WakeByAddressImpl(reinterpret_cast<u32*>(address), count);
        } else if (signal_type == SignalType_SignalAndIncrementIfEqual) {
            return scheduler->WakeByAddressIncrementEqualImpl(reinterpret_cast<u32*>(address), value, count);
        } else if (signal_type == SignalType_SignalAndModifyByWaiterCountIfEqual) {
            return scheduler->WakeByAddressModifyLessThanImpl(reinterpret_cast<u32*>(address), value, count);
        }

        return ResultInvalidSignalType;
    }
}