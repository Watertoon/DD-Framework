#include <dd.hpp>
#include <unit_tester.hpp>

DECLARE_UNIT_TESTER_INSTANCE;

TEST(HandleTableAllocation) {
    
    dd::ukern::HandleTable handle_table = {};
    handle_table.Initialize();

    dd::ukern::UKernHandle handle0 = 0;
    dd::ukern::UKernHandle handle1 = 0;
    dd::ukern::UKernHandle handle2 = 0;
    u32                    value0 = 1;
    u32                    value1 = 2;
    u32                    value2 = 3;

    /* Reserve 3 handles */
    TEST_ASSERT(handle_table.ReserveHandle(std::addressof(handle0), std::addressof(value0)) == true);
    TEST_ASSERT(handle_table.ReserveHandle(std::addressof(handle1), std::addressof(value1)) == true);
    TEST_ASSERT(handle_table.ReserveHandle(std::addressof(handle2), std::addressof(value2)) == true);

    /* Check we get an object back for each handle */
    TEST_ASSERT(handle_table.GetObjectByHandle(handle0) != nullptr);
    TEST_ASSERT(handle_table.GetObjectByHandle(handle1) != nullptr);
    TEST_ASSERT(handle_table.GetObjectByHandle(handle2) != nullptr);

    /* Check we get the correct object back */
    TEST_ASSERT(*reinterpret_cast<u32*>(handle_table.GetObjectByHandle(handle0)) == 1);
    TEST_ASSERT(*reinterpret_cast<u32*>(handle_table.GetObjectByHandle(handle1)) == 2);
    TEST_ASSERT(*reinterpret_cast<u32*>(handle_table.GetObjectByHandle(handle2)) == 3);

    /* Release the handles */
    TEST_ASSERT(handle_table.FreeHandle(handle0) == true);
    TEST_ASSERT(handle_table.FreeHandle(handle1) == true);
    TEST_ASSERT(handle_table.FreeHandle(handle2) == true);

    /* Verify the handles are invalid */
    TEST_ASSERT(handle_table.GetObjectByHandle(handle0) == nullptr);
    TEST_ASSERT(handle_table.GetObjectByHandle(handle1) == nullptr);
    TEST_ASSERT(handle_table.GetObjectByHandle(handle2) == nullptr);

    TEST_SUCCESS;
}

TEST(HandleExhaustion) {

    dd::ukern::HandleTable handle_table = {};
    handle_table.Initialize();

    constexpr u32 MaxHandles     = dd::ukern::HandleTable::MaxHandles;
    constexpr u32 TestIterations = 2048;

    dd::ukern::UKernHandle handles[MaxHandles + 1] = {0};

    /* Run the test for multiple iterations to ensure freeing handles work */
    for (u32 y = 0; y < TestIterations; ++y) {

        /* Reserve all handles */
        u32 values[MaxHandles + 1] = {0};
        for (u32 i = 0; i < MaxHandles; ++i) {
            values[i] = i + 1;
            TEST_ASSERT(handle_table.ReserveHandle(std::addressof(handles[i]), std::addressof(values[i])) == true);
        }

        /* Try to obtain a handle after exhasustion */
        values[MaxHandles] = MaxHandles + 1;
        TEST_ASSERT(handle_table.ReserveHandle(std::addressof(handles[MaxHandles]), std::addressof(values[MaxHandles])) == false);

        /* Ensure each handle points to the correct object */
        for (u32 i = 0; i < MaxHandles; ++i) {
            TEST_ASSERT(handle_table.GetObjectByHandle(handles[i]) != nullptr);
        }
        for (u32 i = 0; i < MaxHandles; ++i) {
            TEST_ASSERT(*reinterpret_cast<u32*>(handle_table.GetObjectByHandle(handles[i])) == (i + 1));
        }

        /* Free all the handles */
        for (u32 i = 0; i < MaxHandles; ++i) {
            TEST_ASSERT(handle_table.FreeHandle(handles[i]) == true);
        }
    }

    TEST_SUCCESS;
}