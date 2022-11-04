#pragma once

namespace dd::ukern {

    class BusyMutex {
        private:
        union {
            struct {
                u16 m_release_count;
                u16 m_lock_count;
            };
            u32 m_counter;
        };
        public:
            constexpr ALWAYS_INLINE BusyMutex() : m_counter(0) {/*...*/}

            ALWAYS_INLINE void Enter() {

                /* Increment lock count */
                u32 wait = ::InterlockedExchangeAdd(reinterpret_cast<volatile long int*>(std::addressof(m_counter)), 0x1'0000);

                /* Wait until release count is matched */
                while ((wait & 0xffff) != ((wait >> 0x10) & 0xffff)) {

                    /* Signal the processor to not aggressively speculatively execute for a bit */
                    util::x64::pause();

                    /* Atomicly acquire the lock and release counters */
                    wait = ::InterlockedAdd(reinterpret_cast<volatile long int*>(std::addressof(m_counter)), 0);
                }
            }

            ALWAYS_INLINE void Leave() {
                /* Increment release */
                ++m_release_count;
            }
    };

    class ScopedBusyMutex {
        private:
            BusyMutex *m_mutex;
        public:
            ALWAYS_INLINE ScopedBusyMutex(BusyMutex *mutex) : m_mutex(mutex) {
                m_mutex->Enter();
            }
            ALWAYS_INLINE ~ScopedBusyMutex() {
                m_mutex->Leave();
            }
    };
}

