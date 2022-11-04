#pragma once

namespace dd::ukern {

    enum  ArbitrationType {
        ArbitrationType_WaitIfEqual,
        ArbitrationType_DecrementWaitIfLessThan,
        ArbitrationType_WaitIfLessThan,
    };
    enum  SignalType {
        SignalType_Signal,
        SignalType_SignalAndIncrementIfEqual,
        SignalType_SignalAndModifyByWaiterCountIfEqual
    };
    
    Result ArbitrateLock(UKernHandle handle, uintptr_t address, u32 tag);
    Result ArbitrateUnlock(uintptr_t address);
    
    Result WaitKey(uintptr_t address, uintptr_t cv_key, u32 tag, s64 timeout_ns);
    Result SignalKey(uintptr_t cv_key, u32 count);

    Result WaitOnAddress(uintptr_t address, u32 arbitration_type, u32 value, s64 timeout_ns);
    Result WakeByAddress(uintptr_t address, u32 signal_type, u32 value, u32 count);
}
