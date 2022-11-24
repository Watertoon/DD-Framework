#pragma once

namespace dd::sys {

    class Event {
        private:
            u16                                    m_signal_state;
            u16                                    m_auto_reset_mode;
            u32                                    m_wake_id;
            ukern::impl::InternalCriticalSection   m_cs;
            ukern::impl::InternalConditionVariable m_cv;
        public:
            constexpr ALWAYS_INLINE Event() {/*...*/}
            constexpr ALWAYS_INLINE Event(bool create_signaled, bool auto_reset) : m_signal_state(create_signaled), m_auto_reset_mode(auto_reset) {/*...*/}

            void Signal() {
                std::scoped_lock lock(m_cs);

                /* If we auto reset, we can only signal one waiter */
                if (m_auto_reset_mode == 1) {
                    m_cv.Signal();
                } else {
                    /* Increment wait id so all previous waiters wake even if the signal is reset */
                    ++wake_id;
                    m_cv.Broadcast();
                }
            }

            void Clear() {
                std::scoped_lock lock(m_cs);
                signal_state = 0;
            }

            void Wait() {
                std::scoped_lock lock(m_cs);

                /* Bail if already signaled */
                if (signal_state == 1) { return; }

                /* Wait until wait is changes or the event is signaled */
                const u32 wait_id = m_wait_id;
                do {
                    m_cv.Wait(std::addressof(m_cs));
                } while (wait_id == m_wait_id && signal_state == 0);

                /* Clear signal if auto reset is enabled */
                if (m_auto_reset_mode == 1) {
                    m_signal_state = 0;
                }

                return;
            }

            void TimedWait(TimeSpan timeout_ns) {
                
            }
    };
}
