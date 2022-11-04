#include <dd.hpp>

namespace dd::ukern::impl {

    void StopAllOtherCores() {
       impl::UserScheduler *scheduler = impl::GetScheduler();
        scheduler->SuspendAllOtherCoresImpl();
    }
    
    void OutputBackTraceToFileAll(HANDLE file) {
       impl::UserScheduler *scheduler = impl::GetScheduler();
        scheduler->OutputBackTraceImpl(file);
    }
}
