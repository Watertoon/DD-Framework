#pragma once

namespace dd::ukern {
    
    DECLARE_RESULT_MODULE(1);
    DECLARE_RESULT(Timeout, 1);
    DECLARE_RESULT(InvalidHandle, 1);
    DECLARE_RESULT(Win32Error, 1);
    DECLARE_RESULT(ThreadStorageExhaustion, 1);
    DECLARE_RESULT(HandleExhaustion, 1);
    DECLARE_RESULT(InvalidThreadFunctionPointer, 1);
    DECLARE_RESULT(InvalidStackSize, 1);
    DECLARE_RESULT(InvalidPriority, 1);
    DECLARE_RESULT(InvalidAddress, 1);
    DECLARE_RESULT(InvalidWaitAddressValue, 1);
    DECLARE_RESULT(InvalidLockAddressValue, 1);
    DECLARE_RESULT(RequiresLock, 1);
    DECLARE_RESULT(ValueOutOfRange, 1);
    DECLARE_RESULT(SamePriority, 1);
    DECLARE_RESULT(SameActivityLevel, 1);
    DECLARE_RESULT(SameCoreMask, 1);
    DECLARE_RESULT(InvalidCoreId, 1);
    DECLARE_RESULT(InvalidArbitrationType, 1);
    DECLARE_RESULT(InvalidSignalType, 1);
}