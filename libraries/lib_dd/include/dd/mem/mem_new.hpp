#pragma once


namespace dd::mem::impl {

    constexpr ALWAYS_INLINE size_t DefaultNewAlignment = 8;
    constexpr ALWAYS_INLINE bool   ForceUseHeapAllocator  = false;

    ALWAYS_INLINE void *NewImpl(size_t size, u32 alignment) {

        /* Attempt to fallback to malloc if the heap manager is not initialized */
        if constexpr (ForceUseHeapAllocator == false) {
            if (dd::mem::IsHeapManagerInitialized() == false) {
                return ::_aligned_malloc(size, alignment);
            }
        }

        /* Try to allocate from current heap if able */
        dd::mem::Heap *current_heap = dd::mem::GetCurrentThreadHeap();
        if (current_heap == nullptr) {
            return nullptr;
        }

        return current_heap->TryAllocate(size, alignment);
    }

    ALWAYS_INLINE void *NewImpl(size_t size, u32 alignment, Heap *heap) {

        /* Attempt to fallback to malloc if the heap manager is not initialized */
        if constexpr (ForceUseHeapAllocator == false) {
            if (dd::mem::IsHeapManagerInitialized() == false) {
                return ::_aligned_malloc(size, alignment);
            }
        }

        /* Try to allocate from specified heap if able, or current heap */
        dd::mem::Heap *current_heap = (heap != nullptr) ? heap : dd::mem::GetCurrentThreadHeap();
        if (current_heap == nullptr) {
            return nullptr;
        }

        return current_heap->TryAllocate(size, alignment);
    }

    ALWAYS_INLINE void DeleteImpl(void *address) {
        if constexpr (ForceUseHeapAllocator == false) {
            if (dd::mem::IsHeapManagerInitialized() == false) {
                return ::_aligned_free (address);
            }
        }
        dd::mem::Heap *current_heap = dd::mem::GetCurrentThreadHeap();
        if (current_heap == nullptr) {
            return;
        }

        current_heap->Free(address);
    }
}

/* Default overloads */

ALWAYS_INLINE void *operator new(size_t size) {
    return dd::mem::impl::NewImpl(size, dd::mem::impl::DefaultNewAlignment);
}

ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment) {
    return dd::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new(size_t size, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, dd::mem::impl::DefaultNewAlignment);
}

ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new[](size_t size) {
    return dd::mem::impl::NewImpl(size, dd::mem::impl::DefaultNewAlignment);
}

ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment) {
    return dd::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

ALWAYS_INLINE void *operator new[](size_t size, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, dd::mem::impl::DefaultNewAlignment);
}

ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, static_cast<u32>(alignment));
}

/* Custom */

ALWAYS_INLINE void *operator new(size_t size, dd::mem::Heap *heap, u32 alignment) {
    return dd::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new(size_t size, dd::mem::Heap *heap, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, dd::mem::impl::DefaultNewAlignment, heap);
}

ALWAYS_INLINE void *operator new(size_t size, dd::mem::Heap *heap, u32 alignment, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, dd::mem::Heap *heap, u32 alignment) {
    return dd::mem::impl::NewImpl(size, alignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, dd::mem::Heap *heap, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, dd::mem::impl::DefaultNewAlignment, heap);
}

ALWAYS_INLINE void *operator new[](size_t size, dd::mem::Heap *heap, u32 alignment, const std::nothrow_t&) {
    return dd::mem::impl::NewImpl(size, alignment, heap);
}

/* Default overloads */

ALWAYS_INLINE void operator delete(void *address) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, std::align_val_t) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, size_t) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, size_t, std::align_val_t) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, std::align_val_t, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, std::align_val_t) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, size_t) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, size_t, std::align_val_t) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, std::align_val_t, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

/* Custom */

ALWAYS_INLINE void operator delete(void *address, dd::mem::Heap *, u32) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, dd::mem::Heap *, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete(void *address, dd::mem::Heap *, u32, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, dd::mem::Heap *, u32) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, dd::mem::Heap *, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}

ALWAYS_INLINE void operator delete[](void *address, dd::mem::Heap *, u32, const std::nothrow_t&) {
    dd::mem::impl::DeleteImpl(address);
}
