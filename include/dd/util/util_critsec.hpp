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

            void Leave() {
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
