#include <dd.hpp>
#include <unit_tester.hpp>

DECLARE_UNIT_TESTER_INSTANCE;

TEST(HeapManagerInitialization) {

    /* Init timestamp */
    dd::util::InitializeTimeStamp();

    /* Set core mask */
    dd::ukern::UKernCoreMask core_mask = 1;

    /* Initialize scheduler */
    dd::ukern::InitializeUKern(core_mask);

    /* Initialize System Manager */
    dd::sys::InitializeSystemManager();

    /* Initialize and finalize heap manager */
    dd::mem::InitializeHeapManager(dd::util::Size32MB);

    /* Find root heap */
    dd::mem::Heap *heap = dd::mem::FindHeapByName("HeapManager::sRootHeap");
    TEST_ASSERT(heap != nullptr);
    TEST_ASSERT(::strcmp(heap->GetName(), "HeapManager::sRootHeap") == 0);

    /* Finalize */
    dd::mem::FinalizeHeapManager();

    TEST_SUCCESS;
}
