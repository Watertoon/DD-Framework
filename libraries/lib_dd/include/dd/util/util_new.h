#pragma once

namespace dd::mem {
    class Heap;
}

ALWAYS_INLINE void *operator new(size_t size);
ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment);
ALWAYS_INLINE void *operator new(size_t size, const std::nothrow_t&);
ALWAYS_INLINE void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&);

ALWAYS_INLINE void *operator new[](size_t size);
ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment);
ALWAYS_INLINE void *operator new[](size_t size, const std::nothrow_t&);
ALWAYS_INLINE void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&);

/* Custom */
ALWAYS_INLINE void *operator new(size_t size, dd::mem::Heap *heap, u32 alignment);
ALWAYS_INLINE void *operator new(size_t size, dd::mem::Heap *heap, const std::nothrow_t&);
ALWAYS_INLINE void *operator new(size_t size, dd::mem::Heap *heap, u32 alignment, const std::nothrow_t&);
ALWAYS_INLINE void *operator new[](size_t size, dd::mem::Heap *heap, u32 alignment);
ALWAYS_INLINE void *operator new[](size_t size, dd::mem::Heap *heap, const std::nothrow_t&);
ALWAYS_INLINE void *operator new[](size_t size, dd::mem::Heap *heap, u32 alignment, const std::nothrow_t&);

/* Default overloads */
ALWAYS_INLINE void operator delete(void *address);
ALWAYS_INLINE void operator delete(void *address, std::align_val_t);
ALWAYS_INLINE void operator delete(void *address, size_t);
ALWAYS_INLINE void operator delete(void *address, size_t, std::align_val_t);
ALWAYS_INLINE void operator delete(void *address, const std::nothrow_t&);
ALWAYS_INLINE void operator delete(void *address, std::align_val_t, const std::nothrow_t&);

ALWAYS_INLINE void operator delete[](void *address);
ALWAYS_INLINE void operator delete[](void *address, std::align_val_t);
ALWAYS_INLINE void operator delete[](void *address, size_t);
ALWAYS_INLINE void operator delete[](void *address, size_t, std::align_val_t);
ALWAYS_INLINE void operator delete[](void *address, const std::nothrow_t&);
ALWAYS_INLINE void operator delete[](void *address, std::align_val_t, const std::nothrow_t&);

/* Custom */
ALWAYS_INLINE void operator delete(void *address, dd::mem::Heap *, u32);
ALWAYS_INLINE void operator delete(void *address, dd::mem::Heap *, const std::nothrow_t&);
ALWAYS_INLINE void operator delete(void *address, dd::mem::Heap *, u32, const std::nothrow_t&);

ALWAYS_INLINE void operator delete[](void *address, dd::mem::Heap *, u32);
ALWAYS_INLINE void operator delete[](void *address, dd::mem::Heap *, const std::nothrow_t&);
ALWAYS_INLINE void operator delete[](void *address, dd::mem::Heap *, u32, const std::nothrow_t&);
