#pragma once

namespace dd::mem {

    struct ExpHeapMemoryBlock {
        u16                     alloc_magic;
        u8                      alignment;
        size_t                  block_size;
        util::IntrusiveListNode exp_list_node;

        static constexpr u16 FreeMagic  = 0x4652;
        static constexpr u16 AllocMagic = 0x414C;

        constexpr ExpHeapMemoryBlock() {/*...*/}
    };

    class ExpHeap final : public Heap {
        public:
            using FreeList      = util::IntrusiveListTraits<ExpHeapMemoryBlock, &ExpHeapMemoryBlock::exp_list_node>::List;
            using AllocatedList = util::IntrusiveListTraits<ExpHeapMemoryBlock, &ExpHeapMemoryBlock::exp_list_node>::List;
        public:
            static constexpr size_t MinimumAlignment             = 4;
            static constexpr size_t MinimumAllocationGranularity = 4;
        private:
            FreeList       m_free_block_list;
            AllocatedList  m_allocated_block_list;
            AllocationMode m_allocation_mode;
        private:
            ALWAYS_INLINE void AddFreeBlock(AddressRange range) {

                void *new_start = range.start;
                void *new_end   = range.end;

                /* Coalesce free blocks if possible */
                for (ExpHeapMemoryBlock &free_block : m_free_block_list) {
                    ExpHeapMemoryBlock *next_block = std::addressof(free_block);

                    if (new_start <= next_block) {
                        if (new_end == next_block) {
                            new_end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(new_end) + next_block->block_size + sizeof(ExpHeapMemoryBlock));
                            next_block->exp_list_node.Unlink();
                        }
                        
                    }
                }

                /* Ensure block size is valid */
                //if (new_end < sizeof(ExpHeapMemoryBlock)) {
                //    return;
                //}
                
                //if ()
                
                /* Add new free block */
                ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(new_start);
                std::construct_at(block);
                block->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
                block->block_size  = static_cast<size_t>(reinterpret_cast<uintptr_t>(new_end) + sizeof(ExpHeapMemoryBlock) - reinterpret_cast<uintptr_t>(new_start));
                
                m_free_block_list.PushBack(*block);
            }
            
            ALWAYS_INLINE void AddUsedBlock(ExpHeapMemoryBlock *free_block, uintptr_t allocation_address, size_t size) {
                
                /* Unlink old free block */
                free_block->exp_list_node.Unlink();
                
                /* Calculate new free block addresses and sizes */
                uintptr_t new_free_address     = allocation_address + size;
                uintptr_t new_free_end_address = reinterpret_cast<uintptr_t>(free_block) + free_block->block_size;
                uintptr_t new_free_total_size  = new_free_end_address - new_free_address;
                
                uintptr_t orig_start_address   = reinterpret_cast<uintptr_t>(free_block) - free_block->alignment;
                uintptr_t used_start_address   = allocation_address - sizeof(ExpHeapMemoryBlock);
                uintptr_t used_alignment       = used_start_address - orig_start_address;
                
                /* Create a new free block at front if alignment is great enough */
                if (sizeof(ExpHeapMemoryBlock) + MinimumAllocationGranularity < used_alignment) {
                    
                    ExpHeapMemoryBlock *front_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(orig_start_address);
                    
                    std::construct_at(front_free_block);
                    front_free_block->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
                    front_free_block->alignment   = 0;
                    front_free_block->block_size  = used_alignment - sizeof(ExpHeapMemoryBlock);

                    m_free_block_list.PushBack(*front_free_block);

                    used_alignment = 0;
                }
                
                /* Create new free block at back if leftover space is great enough */
                if (sizeof(ExpHeapMemoryBlock) + MinimumAllocationGranularity < new_free_total_size) {

                    ExpHeapMemoryBlock *back_free_block = reinterpret_cast<ExpHeapMemoryBlock*>(orig_start_address);
                    
                    std::construct_at(back_free_block);
                    back_free_block->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
                    back_free_block->alignment   = 0;
                    back_free_block->block_size  = new_free_total_size - sizeof(ExpHeapMemoryBlock);

                    m_free_block_list.PushBack(*back_free_block);

                    new_free_total_size = 0;
                }
                
                /* Add used block */
                ExpHeapMemoryBlock *used_block = reinterpret_cast<ExpHeapMemoryBlock*>(used_start_address);

                std::construct_at(used_block);
                used_block->alloc_magic = ExpHeapMemoryBlock::AllocMagic;
                used_block->alignment   = used_alignment;
                used_block->block_size  = size + new_free_total_size;
                
                m_allocated_block_list.PushBack(*used_block);
            }
        protected:
            static ExpHeap *TryCreate(void *address, size_t size, const char *name, bool is_thread_safe) {

                if (address == nullptr || size < (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock) + MinimumAllocationGranularity)) { return nullptr; }

                /* Contruct exp heap object */
                ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(address);
                std::construct_at(new_heap, name, nullptr, address, size, is_thread_safe);

                /* Create and add free node spanning block */
                ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
                std::construct_at(first_block);

                first_block->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
                first_block->block_size = size - sizeof(ExpHeap) - sizeof(ExpHeapMemoryBlock);

                new_heap->m_free_block_list.PushBack(*first_block);

                return new_heap;
            }
        public:
            explicit ExpHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : Heap(name, parent_heap, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start_address) + sizeof(ExpHeap)), size, is_thread_safe) {/*...*/}

            static ExpHeap *TryCreate(size_t size, u32 alignment, const char *name, Heap *parent_heap, bool is_thread_safe) {

                /* Use current heap if one is not provided */
                if (parent_heap == nullptr) {
                    parent_heap = mem::GetCurrentThreadHeap();
                }

                /* Consume largest parent heap free node if size is not provided */
                if (size == 0) {
                    size = parent_heap->GetMaximumAllocatableSize(MinimumAllocationGranularity);
                }

                /* Enforce minimum size */
                if (size < (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock) + MinimumAllocationGranularity)) { return nullptr; }

                /* Allocate heap memory from parent heap */
                void *new_heap_memory = parent_heap->TryAllocate(size, alignment);
                ExpHeap *new_heap = reinterpret_cast<ExpHeap*>(new_heap_memory);
                
                if (new_heap_memory == nullptr) { return nullptr; }

                /* Construct new heap */
                std::construct_at(new_heap, name, parent_heap, new_heap_memory, size, is_thread_safe);

                /* Construct and add free node spanning new heap */
                ExpHeapMemoryBlock *first_block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_heap) + sizeof(ExpHeap));
                std::construct_at(first_block);

                first_block->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
                first_block->block_size = size - (sizeof(ExpHeap) + sizeof(ExpHeapMemoryBlock));

                new_heap->m_free_block_list.PushBack(*first_block);
                
                /* Add to parent heap child list */
                parent_heap->PushBackChild(new_heap);
                
                return new_heap;
            }

            virtual void Finalize() override {/*...*/}

            virtual MemoryRange AdjustHeap() override {
                ScopedHeapLock lock(this);
                ExpHeapMemoryBlock *last_block = std::addressof(m_free_block_list.Back());

                /* Ensure free list is not empty */
                if (m_free_block_list.IsEmpty() == true) {
                    return { m_end_address, 0 };
                }

                /* Ensure the last block encompasses our end address */
                if (reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(last_block) + last_block->block_size + sizeof(ExpHeapMemoryBlock)) != m_end_address) {
                    return { m_end_address, 0 };
                }

                /* Remove block from free list */
                m_free_block_list.Remove(*last_block);

                /* Adjust end address */
                const size_t trimed_size = sizeof(ExpHeapMemoryBlock) + last_block->block_size;
                void *new_end_address = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_end_address) - trimed_size);
                m_end_address = new_end_address;

                /* Resize parent heap memory block */
                if (m_parent_heap != nullptr) {
                    m_parent_heap->AdjustAllocation(m_start_address, trimed_size);
                }

                return { new_end_address, trimed_size };
            }

            virtual size_t AdjustAllocation(void *address, size_t new_size) override {
                ScopedHeapLock lock(this);

                ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
                new_size = util::AlignUp(new_size, MinimumAllocationGranularity);

                /* Nothing to do if the size doesn't change */
                if (block->block_size == new_size) {
                    return new_size;
                }

                /* Reduce range if new size is lesser */
                if (new_size < block->block_size) {
                    this->AddFreeBlock(AddressRange{reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + new_size), reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + block->block_size)});
                    block->block_size = new_size;
                    return new_size;
                }

                /* Find end of block */
                const uintptr_t end_address = reinterpret_cast<uintptr_t>(address) + block->block_size;

                /* Walk free list for the free block directly after this allocation */
                FreeList::iterator free_block = m_free_block_list.begin();
                while (free_block != m_free_block_list.end()) {
    
                    /* Complete if the block is directly after our current free block */
                    if (reinterpret_cast<uintptr_t>(std::addressof((*free_block))) == end_address) {
                        break;
                    }

                    free_block = ++free_block;
                }

                /* Ensure we found the block after and it is large enough */
                ExpHeapMemoryBlock *block_after = std::addressof((*free_block));
                const size_t after_size = block_after->block_size + sizeof(ExpHeapMemoryBlock);
                if (free_block != m_free_block_list.end() && after_size + block->block_size > new_size) { return block->block_size; }

                /* Remove after block from free list */
                block_after->exp_list_node.Unlink();

                /* Find new free node address and size */
                const uintptr_t     new_end_address = reinterpret_cast<uintptr_t>(block_after) + after_size;
                ExpHeapMemoryBlock *new_free        = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) + new_size + sizeof(ExpHeapMemoryBlock));
                const size_t        new_free_size   = new_end_address - reinterpret_cast<uintptr_t>(new_free);

                /* Adjust allocation */
                if (sizeof(ExpHeapMemoryBlock) <= new_free_size) {
                    new_free = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(new_free) - new_free_size);
                }
                block->block_size = reinterpret_cast<uintptr_t>(new_free) - reinterpret_cast<uintptr_t>(address);

                if (sizeof(ExpHeapMemoryBlock) <= new_free_size) {
                    std::construct_at(new_free);

                    new_free->alloc_magic = ExpHeapMemoryBlock::FreeMagic;
                    new_free->alignment   = 0;
                    new_free->block_size  = new_free_size;
                    m_free_block_list.PushBack(*new_free);
                }

                return new_size;
            }

            virtual void *TryAllocate(size_t size, s32 alignment) override {
                ScopedHeapLock lock(this);

                /* Existing free block check */
                if (m_free_block_list.IsEmpty() == true) {
                    return nullptr;
                }

                /* Enforce allocation limits */
                if (alignment < MinimumAlignment) {
                    alignment = MinimumAlignment;
                }

                size = util::AlignUp(size, MinimumAllocationGranularity);

                /* Find a free memory block that corresponds to our desired size */
                uintptr_t allocation_address = 0;
                FreeList::iterator free_block = m_free_block_list.begin();

                /* First fit mode */
                if (m_allocation_mode == AllocationMode_FirstFit) {
                    /* Walk the free list */
                    while (free_block != m_free_block_list.end()) {

                        /* Find the starting address for the allocation */
                        allocation_address = util::AlignUp(reinterpret_cast<uintptr_t>(std::addressof((*free_block))) + sizeof(ExpHeapMemoryBlock), alignment);

                        /* Find the total size of the allocation with alignment */
                        const uintptr_t aligned_allocation_size = allocation_address + size - reinterpret_cast<uintptr_t>(std::addressof((*free_block)));

                        /* Complete if the allocated size is within our free block's range */
                        if (aligned_allocation_size <= (*free_block).block_size) {
                            break;
                        }

                        free_block = ++free_block;
                    }
                } else {
                    /* Best fit mode */
                    size_t smaller_size = SIZE_MAX;
                    uintptr_t smaller_address = 0;
                    ExpHeapMemoryBlock *smaller_block = std::addressof((*m_free_block_list.end()));

                    /* Walk the free list */
                    while (free_block != m_free_block_list.end()) {

                        /* Find the starting address for the allocation */
                        allocation_address = util::AlignUp(reinterpret_cast<uintptr_t>(std::addressof((*free_block))) + sizeof(ExpHeapMemoryBlock), alignment);

                        /* Find the total size of the allocation with alignment */
                        const uintptr_t aligned_allocation_size = allocation_address + size - reinterpret_cast<uintptr_t>(std::addressof((*free_block)));

                        /* Check if we've found a smaller block that fits */
                        if ((*free_block).block_size < smaller_size && aligned_allocation_size <= (*free_block).block_size) {
                            smaller_size    = (*free_block).block_size;
                            smaller_block   = std::addressof((*free_block));
                            smaller_address = allocation_address;

                            /* Break if our size requirements are exact */
                            if ((*free_block).block_size == size) {
                                break;
                            }
                        }

                        free_block = ++free_block;
                        allocation_address = smaller_address;
                    }

                    free_block = m_free_block_list.IteratorTo(*smaller_block);
                }

                /* Ensure we found a block */
                if (free_block != m_free_block_list.end()) { return nullptr; }

                /* Convert our new allocation to a used block */
                this->AddUsedBlock(std::addressof((*free_block)), allocation_address, size);

                return reinterpret_cast<void*>(allocation_address);
            }

            virtual void Free(void *address) override {
                ScopedHeapLock lock(this);

                /* Unlink used block */
                ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
                block->exp_list_node.Unlink();

                /* Add back to free list */
                void *start = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(block) - block->alignment);
                void *end   = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + block->block_size);
                this->AddFreeBlock(AddressRange{start, end});
            }

            virtual size_t GetTotalFreeSize() const override {
                ScopedHeapLock lock(this);

                /* Sum free block sizes */
                size_t free_size = 0;
                for (const ExpHeapMemoryBlock &block : m_free_block_list) {
                    free_size += block.block_size;
                }

                return free_size;
            }

            virtual size_t GetMaximumAllocatableSize(s32 alignment) const override {
                ScopedHeapLock lock(this);

                /* Find largest contiguous free block */
                size_t max_free_size = 0;
                for (const ExpHeapMemoryBlock &block : m_free_block_list) {
                    const uintptr_t start_address = reinterpret_cast<uintptr_t>(std::addressof(block)) + sizeof(ExpHeapMemoryBlock);
                    const uintptr_t end_address   = start_address + block.block_size;
                    const size_t    block_size    = end_address - util::AlignUp(start_address, alignment);
                    if (max_free_size < block_size) {
                        max_free_size = block_size;
                    }
                }

                return max_free_size;
            }

            static size_t GetAllocationSize(void *address) {
                const ExpHeapMemoryBlock *block = reinterpret_cast<ExpHeapMemoryBlock*>(reinterpret_cast<uintptr_t>(address) - sizeof(ExpHeapMemoryBlock));
                return block->block_size;
            }
    };
}
