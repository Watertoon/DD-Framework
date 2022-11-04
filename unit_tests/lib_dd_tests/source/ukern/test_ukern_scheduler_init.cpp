#include <dd.hpp>
#include <unit_tester.hpp>

DECLARE_UNIT_TESTER_INSTANCE;

TEST(SchedulerInit) {

    /* Query win32 core mask */
    dd::ukern::UKernCoreMask core_mask = 3;

    /* Initialize scheduler */
    dd::ukern::InitializeUKern(core_mask);

    TEST_SUCCESS;
}
