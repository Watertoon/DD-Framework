#pragma once

namespace dd::mem {
    
    struct SeparateMemoryBlock {
        size_t                   block_offset;
        size_t                   block_size;
        util::IntrusiveListNode  used_list_node;
    };
    
    class SeparateHeap() final {
        public:
            using SeparateMemoryBlockFreeList = util::IntrusiveListTraits<SeparateMemoryBlock, &SeparateMemoryBlock::used_list_node>::ListType;
        private:
            struct SeparateFreeListHelper {
                SeparateFreeListHelper *next;
                char _dummystorage[sizeof(SeparateMemoryBlock) - sizeof(SeparateFreeListHelper*)];
            };
            static_assert(sizeof(SeparateMemoryBlock) == sizeof(SeparateFreeListHelper));
        private:
            SeparateMemoryBlockFreeList  m_used_block_list;
            SeparateMemoryBlock         *m_management_free_list_start;
            u32                          m_used_management_block_count;
            u32                          m_total_management_block_count;
            u64                          m_management_area_size;
            void                        *m_management_area_start;
        public:
            SeparateHeap(char *name, void *heap_start, u64 block_count, uintptr_t start_offset, u64 max_size, bool is_thread_safe) : Heap(name, nullptr, reinterpret_cast<void*>(start_offset), max_size, is_thread_safe), m_total_management_block_count(block_count), m_management_area_size(block_count * sizeof(SeparateMemoryBlock)) {
                
                
                m_management_area_start = start_offset;
                m_management_free_list_start = start_offset;
                
                /* Initialize managment free list */
                SeparateFreeListHelper *block_list = ;
                for (u32 i = 0; i < m_total_management_block_count - 1; ++i) {
                    block_list[i].next = std::addressof(block_list[i + 1]);
                }
            }
    
            static size_t GetManagementAreaSize(size_t block_count) {
                return block_count * sizeof(SeparateMemoryBlock);
            }
    
            virtual void Finalize() override final {/*...*/}
    
            virtual MemoryRange AdjustHeap() override final {/*...*/}
            virtual size_t AdjustAllocation(void *byte_offset_of_allocation, size_t new_size) override final {/*...*/}
    
            virtual void *TryAllocate(size_t size, size_t alignment) override final {/*...*/}
            virtual void Free(void *byte_offset_of_allocation) override final {/*...*/}
    
            virtual size_t GetTotalSize() override final {/*...*/}
            virtual size_t GetFreeSize() override final {/*...*/}
            virtual size_t GetMaximumAllocatableSize(s32 alignment) override final {/*...*/}
    };
}
