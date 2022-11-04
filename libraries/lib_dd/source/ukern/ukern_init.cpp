#include <dd.hpp>

namespace dd::ukern {

    namespace impl {
        UserScheduler Scheduler = {};

        UserScheduler *GetScheduler() {
            return std::addressof(Scheduler);
        }
    }

    void InitializeUKern(u64 core_mask) {
        impl::Scheduler.Initialize(core_mask);
    }
}