#pragma once

namespace dd::mem {
    
    struct SeparateMemoryBlock {
        size_t                   block_offset;
        size_t                   block_size;
        util::IntrusiveListNode  used_list_node;
    };
    
    class SeparateHeap() final {
        public:
            using SeparateMemoryBlockList = util::IntrusiveListTraits<SeparateMemoryBlock, &SeparateMemoryBlock::used_list_node>::ListType;
        private:
            struct SeparateFreeListHelper {
                SeparateFreeListHelper *next;
                char _dummystorage[sizeof(SeparateMemoryBlock) - sizeof(SeparateFreeListHelper*)];
            };
            static_assert(sizeof(SeparateMemoryBlock) == sizeof(SeparateFreeListHelper));
        private:
            SeparateMemoryBlockList  m_used_block_list;
            SeparateFreeListHelper  *m_management_free_list_start;
            u32                      m_used_management_block_count;
            u32                      m_total_management_block_count;
            u64                      m_management_area_size;
            void                    *m_management_area_start;
        public:
            SeparateHeap(char *name, void *heap_start, u64 heap_size, void *management_start, u64 management_size, bool is_thread_safe) : Heap(name, nullptr, reinterpret_cast<void*>(start_offset), heap_size, is_thread_safe), m_total_management_block_count(block_count), m_management_area_size(block_count * sizeof(SeparateMemoryBlock)) {

                m_management_area_start      = management_start;
                m_management_free_list_start = management_start;

                /* Initialize managment free list */
                SeparateFreeListHelper *block_list = reinterpret_cast<SeparateFreeListHelper*>(management_start);
                for (u32 i = 0; i < m_total_management_block_count - 1; ++i) {
                    block_list[i].next = std::addressof(block_list[i + 1]);
                }
            }

            static size_t GetManagementAreaSize(size_t block_count) {
                return block_count * sizeof(SeparateMemoryBlock);
            }

            static SeperateHeap *Create(const char *name, void *work_memory_start, u64 heap_size, u64 management_size, bool is_thread_safe) {

                /* Construct new seperate heap */
                SeperateHeap *heap = reinterpret_cast<SeperateHeap*>(work_memory_start);
                std::construct_at(heap, name, 0x10000, heap_size, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(work_memory_start + sizeof(SeparateHeap)), management_area_size, is_thread_safe);

                return heap;
            }
            
            virtual void Finalize() override final {/*...*/}
    
            virtual MemoryRange AdjustHeap() override final {/*...*/}
            virtual size_t AdjustAllocation(void *byte_offset_of_allocation, size_t new_size) override final {/*...*/}
    
            virtual void *TryAllocate(size_t size, s32 alignment) override final {

                /* Integrity checks */
                DD_ASSERT(m_management_free_list_start != nullptr);

                /* Conditional heap lock */
                ScopedHeapLock l(this);

                /* Find new offset */
                
                /* Create new separate heap block with offset */
                SeparateMemoryBlock *new_block = reinterpret_cast<SeparateMemoryBlock*>(m_management_free_list_start);
                m_management_free_list_start = m_management_free_list_start->next;

                new_block->block_offset = new_offset;
                new_block->block_size   = aligned_size;
                prev_block->LinkNext(new_block->used_list_node);
                
                return new_offset;
            }

            virtual void Free(void *byte_offset_of_allocation) override final {

                /* Conditional heap lock */
                ScopedHeapLock l(this);

                /* Find block */
                for (SeparateMemoryBlock &used_block : m_used_block_list) {
                    if (used_block.block_offset == byte_offset_of_allocation) {

                        /* Swap used block to free list */
                        used_block.used_list_node.Unlink();
                        reinterpret_cast<SeparateFreeListHelper*>(used_block)->next = m_management_free_list_start;
                        m_management_free_list_start = reinterpret_cast<SeparateFreeListHelper*>(used_block)->next;
                    }
                }
            }
    
            virtual size_t GetTotalFreeSize() override final {
                
            }

            virtual size_t GetMaximumAllocatableSize(s32 alignment) override final {
                
            }
    };
}
