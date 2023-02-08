#pragma once

namespace dd::sys {
    
    class DelegateThread : public Thread {
        private:
            util::IDelegate2<DelegateThread*, size_t> *m_delegate;
        private:

            virtual void ThreadCalc(size_t message) override {
                m_delegate->Invoke(this, message);
            }
        public:
            explicit DelegateThread(util::IDelegate2<DelegateThread*, size_t> *delegate, const char *name, mem::Heap *heap, ThreadRunMode run_mode, u32 stack_size, size_t exit_code, u32 max_messages, u32 priority) : Thread(name, heap, run_mode, exit_code, max_messages, stack_size, priority) {
                DD_ASSERT(delegate != nullptr);

                m_delegate = delegate;
            }
    };
}
