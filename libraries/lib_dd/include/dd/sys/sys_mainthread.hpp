#pragma once

namespace dd::sys {
    
    class MainThread : public ThreadBase {
        public:
            MainThread(mem::Heap *thread_heap) : ThreadBase(thread_heap) {
                
            }
    };
}
