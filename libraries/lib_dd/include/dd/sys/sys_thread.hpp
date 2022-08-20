#pragma once

namespace dd::sys {
    
    class Thread {
        private:
            ukern::ThreadType       *m_thread_type;
            ukern::UKernHandle       m_thread_handle;
            sys::Heap               *m_heap;
            util::MessageQueue       m_message_queue;
            size_t                  *m_message_queue_buffer;
            size_t                   m_exit_code;
            ThreadRunMode            m_run_mode;
            util::IntrusiveListNode  m_thread_manager_list_node;
        private:
            static unsigned long InternalThreadMain(void *arg);

            void Run();
            
            virtual void ThreadCalc([[maybe_unused]] void *message) {/*...*/}
        public:
            explicit Thread(const char *name, Heap *heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority);
            explicit Thread(const char *name, Heap *heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority, s32 core_mask);
            explicit Thread(const char *name, Heap *heap, ThreadType *main_thread);
            
            void StartThread() { ukern::StartThread(m_thread_handle); }
            void ExitThread()  {

                /* Remove from thread manager */
                
                /* Exit UKern thread */
                ukern::ExitThread(m_thread_handle);
            }
            
            void SuspendThread() { ukern::SuspendThread(m_thread_handle); }

            void SendMessage(size_t message) { m_message_queue.SendMessage(message); }
            
            constexpr ALWAYS_INLINE Heap *GetThreadHeap() { return m_heap; }
            constexpr ALWAYS_INLINE const Heap *GetThreadHeap() const { return m_heap; }
    };
}