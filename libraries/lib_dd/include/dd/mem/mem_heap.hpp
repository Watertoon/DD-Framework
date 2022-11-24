#pragma once

namespace dd::mem {

    enum HeapType : u8 {
        HeapType_Heap         = 0,
        HeapType_ExpHeap      = 1,
        HeapType_UnitHeap     = 2,
        HeapType_FrameHeap    = 3,
        HeapType_SeperateHeap = 4
    };

    enum AllocationMode : bool {
        AllocationMode_FirstFit = 0,
        AllocationMode_BestFit  = 1
    };

    struct MemoryRange {
        void   *address;
        size_t  size;
    };

    struct AddressRange {
        void *start;
        void *end;
        
        ptrdiff_t GetSize() const { return reinterpret_cast<intptr_t>(start) - reinterpret_cast<intptr_t>(end); }
    };

    class Heap;

    class ScopedHeapLock {
        private:
            Heap *m_heap;
        public:
            explicit constexpr ScopedHeapLock(const Heap *heap);
            constexpr ~ScopedHeapLock();
    };

    class Heap {
        public:
            friend class IDisposer;
            friend class ScopedHeapLock;
            friend Heap *FindHeapByNameImpl(Heap *parent_heap, const char *heap_name);
        public:
            using DisposerList              = util::IntrusiveListTraits<IDisposer, &IDisposer::m_disposer_list_node>::List;
        protected:
            void                           *m_start_address;
            void                           *m_end_address;
            Heap                           *m_parent_heap;
            util::IntrusiveListNode         m_child_list_node;
        protected:
            using ChildList                 = util::IntrusiveListTraits<Heap, &Heap::m_child_list_node>::List;
        protected:
            ChildList                       m_child_list;
            DisposerList                    m_disposer_list;
            const char                     *m_name;
            ukern::InternalCriticalSection  m_heap_cs;
            bool                            m_is_thread_safe;
        private:
            void AppendDisposer(IDisposer &disposer) {
                ScopedHeapLock lock(this);
                m_disposer_list.PushBack(disposer);
            }

            void RemoveDisposer(IDisposer &disposer) {
                ScopedHeapLock lock(this);
                m_disposer_list.Remove(disposer);
            }
        public:
            void PushBackChild(Heap *child) {
                ScopedHeapLock lock(this);
                m_child_list.PushBack(*child);
            }
        public:
            explicit Heap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : m_start_address(start_address), m_end_address(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start_address) +  size)), m_parent_heap(parent_heap), m_name(name), m_is_thread_safe(is_thread_safe) {/*...*/}

            virtual void Finalize();

            virtual MemoryRange AdjustHeap();
            virtual size_t AdjustAllocation(void *address, size_t new_size);

            virtual void *TryAllocate(size_t size, s32 alignment);

            virtual void Free(void *address);

            virtual Heap *FindHeapFromAddress(void *address) {

                /* Check if address is in this heap */
                if (this->IsAddressInHeap(address) == false) { return nullptr; }

                /* Walk the child heap list to determine if the address is in a child heap */
                for (Heap &child : m_child_list) {
                    if (child.IsAddressInHeap(address) == true) {
                        /* Repeat for child heap */
                        return child.FindHeapFromAddress(address);
                    }
                }

                /* Return this heap if address is part of heap, yet not part of a child heap */
                return this;
            }

            virtual bool IsAddressInHeap(void *address) { return (m_start_address <= address && address < m_end_address); }

            virtual size_t GetTotalSize() const { 
                return reinterpret_cast<uintptr_t>(m_end_address) - reinterpret_cast<uintptr_t>(m_start_address);
            }
            virtual size_t GetTotalFreeSize() const;
            virtual size_t GetMaximumAllocatableSize(s32 alignment) const;

            constexpr ALWAYS_INLINE const char *GetName() const { return m_name; }
            constexpr ALWAYS_INLINE bool HasChldren() const { return m_child_list.IsEmpty(); }
    };

    constexpr ALWAYS_INLINE ScopedHeapLock::ScopedHeapLock(const Heap *heap) : m_heap(const_cast<Heap*>(heap)) {
        if (m_heap->m_is_thread_safe == true) {
            m_heap->m_heap_cs.Enter();
        }
    }
    constexpr ALWAYS_INLINE ScopedHeapLock::~ScopedHeapLock() {
        if (m_heap->m_is_thread_safe == true) {
            m_heap->m_heap_cs.Leave();
        }
    }
}
