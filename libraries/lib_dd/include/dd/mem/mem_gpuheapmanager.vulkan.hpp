#pragma once

namespace dd::mem {
    
    enum GpuMemoryType {
        
    };

    class GpuMemoryAddress {
        private:
            GpuMemoryAllocation *m_gpu_allocation;
            size_t               m_offset;
        public:
            constexpr ALWAYS_INLINE explicit GpuMemoryAddress(GpuMemoryAllocation allocation, size_t offset) : m_gpu_allocation(allocation), m_offset(offset) {/*...*/}

            VkBuffer CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size) {
                return m_gpu_allocation->CreateBuffer(usage_flags, size, m_offset);
            }
            
            VkDeviceAddress GetAddress() {
                return m_gpu_allocation->GetAddress() + m_offset;
            }
    };

    class GpuMemoryAllocation {
        private:
            size_t                   m_size;
            u32                      m_offset;
            u16                      m_memory_type;
            u16                      m_is_dedicated_allocation;
            GpuHeapMemory           *m_parent_gpu_heap_memory;
            util::IntrusiveListNode  m_gpu_memory_allocation_list_node;
        public:
            constexpr GpuMemory() {/*...*/}

            Result TryAllocateGpuMemory(mem::Heap meta_data_heap, size_t size, s32 alignment, u32 memory_type) {

                /* Allocate from heap manager */
                return GpuMemoryHeapManager::GetInstance()->TryAllocateGpuMemory(std::addressof(this), meta_data_heap, size, alignment, memory_type);
            }

            void FreeGpuMemory() {

                /* Free from gpu heap memory */
                m_parent_gpu_heap_memory->FreeAllocation();
            }
            
            VkBuffer CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size = VK_WHOLE_SIZE, size_t offset = 0) {

                /* Create buffer */
                VkBuffer buffer = 0;
                const VkBufferCreateInfo buffer_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = size,
                    .usage = usage_flags | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                };
                const u32 result0 = ::pfn_vkCreateBuffer(vk::Context::GetInstance()->GetVkDevice(), std::addressof(buffer_info), nullptr, std::addressof(buffer));
                DD_ASSERT(result0 == VK_SUCCESS);

                const u32 result1 = ::pfn_vkBindBufferMemory(DriverManager::GetInstance()->GetDevice(), buffer, m_vk_device_memory, offset);
                DD_ASSERT(result1 == VK_SUCCESS);

                return buffer;
            }

            constexpr ALWAYS_INLINE GpuMemoryAddress GetGpuAddress(size_t offset) {
                const GpuMemoryAddress address(this, offset);
                return address;
            }

            constexpr ALWAYS_INLINE VkDeviceAddress GetDeviceAddress() {
                return m_vk_device_address;
            }

            void FlushCache(size_t size);
    };
    
    class GpuHeapMemory {
        private:
            using GpuMemoryAllocationList = util::IntrusiveListTraits<GpuMemoryAllocation, &GpuMemoryAllocation::m_gpu_memory_allocation_list_node>::List;
        private:
            GpuHeap                 *m_parent_heap;
            VkDeviceMemory           m_vk_device_memory;
            size_t                   m_memory_size;
            u32                      m_memory_type;
            mem::SeparateHeap       *m_gpu_separate_heap;
            GpuMemoryAllocationList  m_gpu_memory_allocation_list;
            util::IntrusiveListNode  m_gpu_heap_memory_list_node;
        public:
            constexpr ALWAYS_INLINE GpuHeapMemory() {/*...*/}

            static GpuHeapMemory *Create(GpuHeap *parent_heap, mem::heap *heap, size_t size, s32 alignment, u32 memory_type) {
                
                /* Find block count */
                const u32 block_count = 32;
                
                /* Allocate new GpuHeapMemory and Separate Heap */
                GpuHeapMemory *gpu_heap_memory = reinterpret_cast<GpuHeapMemory*>(::operator new(sizeof(GpuHeapMemory) + sizeof(mem::SeparateHeap) + SeparateHeap::GetManagementAreaSize(block_count), heap, 8));
                std::construct_at(gpu_heap_memory);

                /* Create separate heap */
                
                gpu_heap_memory->m_gpu_separate_heap = mem::SeparateHeap::Create("gpuheap", work_memory, size, SeparateHeap::GetManagementAreaSize(block_count), false);

                return gpu_heap_memory;
            }
            
            Result TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, size_t size, s32 alignment, u32 memory_type) {

                /* Allocate from separate heap */
                void *alloc = m_gpu_separate_heap->TryAllocate(size, alignment);
                DD_ASSERT(alloc != alloc);

                out_allocation->m_offset                 = reinterpret_cast<uintptr_t>(alloc) - mem::SeperateHeap::OffsetBase;
                out_allocation->m_size                   = size;
                out_allocation->m_memory_type            = memory_type;
                out_allocation->m_parent_gpu_heap_memory = this;
                m_gpu_memory_allocation_list.PushBack(*out_allocation);
            }
    };
    
    class GpuHeap : public mem::IDisposer {
        private:
            using GpuHeapMemoryList = util::IntrusiveListTraits<GpuHeapMemory, &GpuHeapMemory::m_gpu_heap_memory_list_node>::List;
        private:
            GpuHeapMemoryList m_gpu_heap_memory_list;
            sys::Mutex        m_heap_memory_list_mutex;
        public:
            constexpr ALWAYS_INLINE GpuHeap(mem::Heap heap) : IDisposer(heap) {/*...*/}
            virtual constexpr ALWAYS_INLINE ~GpuHeap() {/*...*/}

            Result TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap heap, size_t size, s32 alignment, u32 memory_type, size_t minimum_size, size_t maximum_size) {

                /* Find valid GpuHeapMemory */
                GpuHeapMemory *gpu_heap_memory = nullptr;
                {
                    std::scoped_lock lock(m_heap_memory_list_mutex);

                    for (GpuHeapMemory &memory_iter : m_gpu_heap_memory_list) {
                        if ((memory_iter.m_memory_type & memory_type) == memory_type && size < memory_iter.m_gpu_separate_heap->GetMaxAllocatableSize(alignment)) {
                            gpu_heap_memory = std::addressof(memory_iter);
                            break;
                        }
                    }
                }

                /* Create new GpuHeapMemory if needed */
                if (gpu_heap_memory == nullptr) {
                    gpu_heap_memory = GpuHeapMemory::Create(this, size, alignment, memory_type);
                    m_gpu_heap_memory_list.PushBack(*gpu_heap_memory);
                }

                /* Allocate from GpuHeapMemory */
                return gpu_heap_memory->TryAllocateGpuMemory(out_allocation, size, alignment);
            }
    };

    class GpuMemoryHeapManager {
        public:
            static constexpr size_t MaxAllocations = 0x400;
        private:
            util::PointerArray<GpuHeap> m_gpu_heap_array;
            size_t                      m_miminum_block_size;
            sys::Mutex                  m_heap_array_mutex;
        private:
            DD_SINGLETON_TRAITS(GpuMemoryAllocationManager);
        public:
            constexpr GpuMemoryHeapManager() {/*...*/}

            void Initialize(mem::Heap heap, size_t minumum_block_size) {

                /* Initialize pointer array */
                m_gpu_heap_ptr_array.Initialize(heap, MaxAllocations);

                /* Set state */
                m_miminum_block_size = minimum_block_size;
            }

            Result TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap heap, size_t size, s32 alignment, u32 memory_type) {

                /* Use a previous GpuHeap if our current Heap corresponds to one */
                GpuHeap *gpu_heap = nullptr;
                {
                    std::scoped_lock lock(m_heap_array_mutex);

                    for (GpuHeap &heap_iter : m_gpu_heap_array) {
                        if (heap_iter.GetDisposerHeap() == heap) {
                            gpu_heap = std::addressof(heap_iter);
                            break;
                        }
                    }

                    /* Allocate new block if we can't find one */
                    if (gpu_heap == nullptr) {
                        RESULT_RETURN_UNLESS(m_gpu_heap_array.GetCount() < m_gpu_array.GetMaxCount(), ResultMaximumGpuHeaps);

                        gpu_heap = new (heap) GpuHeap(heap);
                        m_gpu_heap_array.PushBack(gpu_heap);
                    }
                }

                /* Allocate from gpu heap */
                return gpu_heap->TryAllocateGpuMemory(out_allocation, heap, size, alignment, memory_type, minimum_block_size, vk::Context::MaximumUniformBufferSize);
            }
    };
}
