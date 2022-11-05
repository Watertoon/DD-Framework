#include <dd.hpp>
#include <unit_tester.hpp>

DECLARE_UNIT_TESTER_INSTANCE;

u64 TestValue = 0;

void TestWaitMain(void *arg) {

    dd::ukern::InternalCriticalSection *cs = reinterpret_cast<dd::ukern::InternalCriticalSection*>(arg);

    /* Acquire the main CS */
    cs->Enter();

    /* Set a value */
    TestValue = 1;

    /* Leave the CS */
    cs->Leave();

    return;
}

TEST(SchedulerSingleCoreThreadValueSet) {

    /* Use a single thread */
    dd::ukern::UKernCoreMask core_mask = 1;

    /* Initialize scheduler */
    dd::ukern::InitializeUKern(core_mask);

    /* Create an internal critical section */
    dd::ukern::InternalCriticalSection cs = {};

    /* Create a thread */
    dd::ukern::UKernHandle handle = 0;
    const u32 result0 = dd::ukern::CreateThread(std::addressof(handle), TestWaitMain, reinterpret_cast<uintptr_t>(std::addressof(cs)), 0x1000, THREAD_PRIORITY_NORMAL, 0);
    ::printf("0x%X\n", result0);
    TEST_ASSERT(result0 != dd::ukern::ResultInvalidThreadFunctionPointer);
    TEST_ASSERT(result0 != dd::ukern::ResultInvalidStackSize);
    TEST_ASSERT(result0 != dd::ukern::ResultInvalidPriority);
    TEST_ASSERT(result0 != dd::ukern::ResultInvalidCoreId);

    /* Enter the CS */
    cs.Enter();
    
    dd::ukern::ThreadType *thread = dd::ukern::GetCurrentThread();
    TEST_ASSERT(thread->current_core == 0);

    /* Start other thread */
    const u32 result1 = dd::ukern::StartThread(handle);
    ::printf("0x%X\n", result1);
    TEST_ASSERT(result1 == dd::ResultSuccess);

    /* Sleep for 1 second */
    dd::ukern::YieldThread();

    /* Assert value hasn't changed */
    TEST_ASSERT(TestValue != 0);

    /* Release CS */
    cs.Leave();

    /* Wait for value to be set */
    while (TestValue != 1) {
        dd::ukern::YieldThread();
    }

    TEST_SUCCESS;
}
