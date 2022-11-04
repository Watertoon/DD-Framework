#pragma once

namespace dd::ukern {

    class HandleTable {
        public:
            static constexpr size_t MaxHandles = 256;
            static constexpr size_t HandleLastIndexBits = 15;
            static constexpr size_t HandleIndexBits = 8;
        private:
            u16        m_indices[MaxHandles];
            void      *m_objects[MaxHandles];
            BusyMutex  m_table_mutex;
            u16        m_active_handles;
            u16        m_last_free_indice;
            u16        m_next_indice;
        public:
            constexpr HandleTable() : m_indices{}, m_objects{} {/*...*/}

            bool ReserveHandle(u32 *out_handle, void *object) {
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                if (MaxHandles <= m_active_handles) { return false; }

                const u16 last_indice = m_last_free_indice;
                m_last_free_indice    = m_indices[last_indice];

                const u16 next_indice = m_next_indice;
                m_indices[last_indice] = next_indice;
                m_objects[last_indice] = object;

                ++m_active_handles;

                if (m_next_indice < 0x7fff) {
                    ++m_next_indice;
                }

                *out_handle = (last_indice & 0x7fff) | (next_indice << 0xf);

                return true;
            }

            bool FreeHandle(u32 handle) {
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                if ((handle == 0) || ((handle >> 0xf) == 0)) { return false; }

                const u32 index = (handle & 0x7fff);
                if (MaxHandles < index || m_objects[index] == nullptr) { return false; }

                m_objects[index] = nullptr;
                m_indices[index] = m_last_free_indice;
                m_last_free_indice = index;
                --m_active_handles;

                return true;
            }

            void *GetObjectByHandle(u32 handle) {
                ScopedBusyMutex lock(std::addressof(m_table_mutex));

                if (((handle >> 0x1e) != 0) || (handle == 0) || ((handle >> 0xf) == 0)) { return nullptr; }

                const u32 index = (handle & 0x7fff);
                if ((MaxHandles < index) || (handle >> 0xf != m_indices[index])) { return nullptr; }

                return m_objects[index];
            }
    };
}
