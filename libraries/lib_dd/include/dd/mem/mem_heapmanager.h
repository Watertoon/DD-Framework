#pragma once

namespace dd::mem {
    
    class Heap;
    
    struct HeapAllocationFailedInfo {
        Heap   *heap;
        size_t  allocation_size;
        size_t  aligned_allocation_size;
        size_t  alignment;
    };

    struct HeapManager {
        bool                                         is_initialized;
        void                                        *memory;
        size_t                                       memory_size;
        util::IDelegate1<HeapAllocationFailedInfo*> *allocate_failed_callback;
    };

    void InitializeHeapManager(size_t size);
    void FinalizeHeapManager();

    HeapManager *GetHeapManager();
    Heap        *GetRootHeap();
    
    bool IsHeapManagerInitialized();

    Heap *FindContainedHeap(void *address);
    Heap *FindHeapByName(const char *heap_name);

    Heap *GetCurrentThreadHeap();
    void  SetCurrentThreadHeap(Heap *heap);

    Heap *FindHeapFromAddress(void *address);
    bool  IsAddressFromAnyHeap(void *address);
}
