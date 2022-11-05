#include <dd.hpp>

namespace dd::ukern {

    namespace impl {
        UserScheduler SchedulerInstance = {};

        UserScheduler *GetScheduler() {
            return std::addressof(SchedulerInstance);
        }
    }

    void InitializeUKern(u64 core_mask) {
        impl::SchedulerInstance.Initialize(core_mask);
    }
}