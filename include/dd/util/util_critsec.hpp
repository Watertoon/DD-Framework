#pragma once

namespace dd::util {

    class CriticalSection {
        private:
            SRWLOCK m_srwlock;
            u32     m_locked_thread_id;
        public:
            constexpr ALWAYS_INLINE CriticalSection() : m_srwlock{0}, m_locked_thread_id(0) {/*...*/}

            void lock() {
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                m_locked_thread_id = ::GetCurrentThreadId();
            }

            void unlock() {
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
            }

            bool try_lock() {
                const bool result = ::TryAcquireSRWLockExclusive(std::addressof(m_srwlock));
                if (result == true) {
                    m_locked_thread_id = ::GetCurrentThreadId();;
                }
            }

            void Enter() {
                return this->lock();
            }

            void Leaver() {
                return this->unlock();
            }

            bool TryEnter() {
                return this->try_lock();
            }

            bool IsLockOwnedByCurrentThread() {
                return ::GetCurrentThreadId() == m_locked_thread_id;
            }
    };
}