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

    //struct UnitHeapMemoryBlock {
    //    UnitHeapMemoryBlock *next;
    //};
    //class UnitHeap : public Heap {
    //    private:
    //        u64 m_block_size;
    //        UnitHeapMemoryBlock *m_free_block_list;
    //    public:
    //        constexpr UnitHeap() {/*...*/}
    //        
    //        virtual void Finalize() override {/*...*/}
    //        
    //        virtual void Adjust() override {/*...*/}
    //        
    //        virtual void *TryAllocate(size_t size, size_t alignment) override {
    //            UnitHeapMemoryBlock *block = m_free_block_list;
    //            if (block != nullptr) {
    //                m_free_block_list = *m_free_block_list;
    //            }
    //            return block;
    //        }
    //        
    //        virtual void Free(void *address) override {
    //            if (address != nullptr) {
    //                *reinterpret_cast<UnitHeapMemoryBlock**>(address) = m_free_block_list;
    //                m_free_block_list = reinterpret_cast<UnitHeapMemoryBlock*>(address);
    //            }
    //        }
    //        
    //        virtual void ResizeFront() override {/*...*/}
    //
    //        virtual void ResizeBack() override {/*...*/}
    //        
    //        virtual size_t GetTotalSize() override {/*...*/}
    //        virtual size_t GetFreeSize() override {/*...*/}
    //        virtual size_t GetMaximumAllocatableSize() override {/*...*/}
    //};

    //struct SeparateMemoryBlock {
    //    size_t                   block_offset;
    //    size_t                   block_size;
    //    util::IntrusiveListNode  used_list_node;
    //};
    //
    //class SeparateHeap() final {
    //    public:
    //        using SeparateMemoryBlockFreeList = util::IntrusiveListTraits<SeparateMemoryBlock, &SeparateMemoryBlock::used_list_node>::ListType;
    //    private:
    //        struct SeparateFreeListHelper {
    //            SeparateFreeListHelper *next;
    //            char _dummystorage[sizeof(SeparateMemoryBlock) - sizeof(SeparateFreeListHelper*)];
    //        };
    //        static_assert(sizeof(SeparateMemoryBlock) == sizeof(SeparateFreeListHelper));
    //    private:
    //        SeparateMemoryBlockFreeList  m_used_block_list;
    //        SeparateMemoryBlock         *m_management_free_list_start;
    //        u32                          m_used_management_block_count;
    //        u32                          m_total_management_block_count;
    //        u64                          m_management_area_size;
    //        void                        *m_management_area_start;
    //    public:
    //        SeparateHeap(char *name, void *heap_start, u64 block_count, uintptr_t start_offset, u64 max_size, bool is_thread_safe) : Heap(name, nullptr, reinterpret_cast<void*>(start_offset), max_size, is_thread_safe), m_total_management_block_count(block_count), m_management_area_size(block_count * sizeof(SeparateMemoryBlock)) {
    //            
    //            
    //            m_management_area_start = start_offset;
    //            m_management_free_list_start = start_offset;
    //            
    //            /* Initialize managment free list */
    //            SeparateFreeListHelper *block_list = ;
    //            for (u32 i = 0; i < m_total_management_block_count - 1; ++i) {
    //                block_list[i].next = std::addressof(block_list[i + 1]);
    //            }
    //        }
    //
    //        static size_t GetManagementAreaSize(size_t block_count) {
    //            return block_count * sizeof(SeparateMemoryBlock);
    //        }
    //
    //        virtual void Finalize() override final {/*...*/}
    //
    //        virtual MemoryRange Adjust() override final {/*...*/}
    //        virtual size_t AdjustAllocation(void *byte_offset_of_allocation, size_t new_size) override final {/*...*/}
    //
    //        virtual void *TryAllocate(size_t size, size_t alignment) override final {/*...*/}
    //        virtual void Free(void *byte_offset_of_allocation) override final {/*...*/}
    //
    //        virtual size_t GetTotalSize() override final {/*...*/}
    //        virtual size_t GetFreeSize() override final {/*...*/}
    //        virtual size_t GetMaximumAllocatableSize(s32 alignment) override final {/*...*/}
    //};
}
