 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

namespace dd::util {

    class DelegateThread {
        private:
            HANDLE                               m_thread_handle;
            MessageQueue                         m_message_queue;
            size_t                               m_exit_code;
            u32                                  m_stack_size;
            long unsigned int                    m_thread_id;
            IDelegate2<DelegateThread*, size_t> *m_delegate;
        private:
            static unsigned long ThreadMain(void *arg) {
                reinterpret_cast<DelegateThread*>(arg)->DelegateThreadMain();
                return 0;
            }

            void DelegateThreadMain() {

                size_t current_message = 0;
                m_message_queue.ReceiveMessage(std::addressof(current_message));

                while(current_message != m_exit_code) {
                    m_delegate->Invoke(this, current_message);
                    m_message_queue.ReceiveMessage(std::addressof(current_message));
                }
            }
        public:
            DelegateThread(IDelegate2<DelegateThread*, size_t> *delegate, u32 stack_size, size_t exit_code, u32 max_messages) {
                DD_ASSERT(delegate != nullptr);

                m_delegate = delegate;
                m_stack_size = stack_size;
                m_exit_code = exit_code;

                m_message_queue.Initialize(max_messages);

                m_thread_handle = ::CreateThread(nullptr, stack_size, ThreadMain, this, 0, std::addressof(m_thread_id));
                DD_ASSERT(m_thread_handle != nullptr);
            }

            void SendMessage(size_t message) {
                m_message_queue.SendMessage(message);
            }

            void FinalizeThread() {
                this->SendMessage(m_exit_code);
                ::WaitForSingleObject(m_thread_handle, INFINITE);
            }

            constexpr size_t GetExitCode() const { return m_exit_code; }
    };
}
