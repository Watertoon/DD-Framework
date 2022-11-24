#include <dd.hpp>

namespace dd::mem {

    namespace {
        constinit util::TypeStorage<HeapManager>  sHeapManagerStorage       = {};
    //    constinit Heap                           *sRootHeap                 = nullptr;
        constinit sys::Mutex                      sHeapManagerMutex         = {};
        constinit bool                            sIsHeapManagerInitialized = false;
    }

    //ALWAYS_INLINE void IntializeHeapManager(size_t size) {
    //    std::scoped_lock l(sHeapManagerMutex);
    //
    //    util::ConstructAt(sHeapManagerStorage);
    //    HeapManager *heap_mgr = util::GetPointer(sHeapManagerStorage);
    //
    //    /* Query our allocation granularity */
    //    size_t allocation_granularity = sys::GetSystemInfo()->dwAllocationGranularity;
    //
    //    /* Create our program arena */
    //    heap_mgr->memory = ::VirtualAlloc(nullptr, util::AlignUp(size, allocation_granularity), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    //    DD_ASSERT(heap_mgr->memory != nullptr);
    //
    //    heap_mgr->memory_size = size;
    //
    //    /* Create our root heap spanning the arena */
    //    sRootHeap = ExpHeap::TryCreate(heap_mgr->memory, heap_mgr->memory_size, "HeapManager::sRootHeap", false);
    //    
    //    sIsHeapManagerInitialized = true;
    //}
    //
    //ALWAYS_INLINE void FinalizeHeapManager() {
    //    std::scoped_lock l(sHeapManagerMutex);
    //
    //    /* Destroy root heap */
    //    sRootHeap->Finalize();
    //
    //    /* Free our program's memory */
    //    HeapManager *heap_mgr = util::GetPointer(sHeapManagerStorage);
    //
    //    bool result = ::VirtualFree(heap_mgr->memory, heap_mgr->m_memory_size, MEM_DECOMMIT | MEM_RELEASE);
    //    DD_ASSERT(result == true);
    //    heap_mgr->memory = nullptr;
    //    heap_mgr->memory_size = 0;
    //}

    //Heap *FindContainedHeap(void *address) {
    //
    //    /* Try to acquire thread */
    //    sys::impl::ThreadManager *thread_manager = sys::impl::ThreadManager::GetInstance();
    //
    //    sys::Thread *thread = nullptr;
    //    if (thread_manager != nullptr) { thread = thread_manager->GetCurrentThread(); }
    //
    //    /* Try to lookup address in the thread heaps */
    //    Heap *thread_heap      = nullptr;
    //    Heap *last_lookup_heap = nullptr;
    //    if (thread != nullptr) {
    //        thread_heap      = thread->GetCurrentHeap();
    //        last_lookup_heap = thread->GetLookupHeap();
    //
    //        /* Check lookup heap to see if we have no children and contain the address*/
    //        if (last_lookup_heap != nullptr && last_lookup_heap->HasChildren() == false && last_lookup_heap->IsAddressInHeap(address) == true) {
    //            return last_lookup_heap;
    //        }
    //
    //        /* Check thread's current heap to see if we have no children and contain the address */
    //        if (thread_heap != nullptr && thread_heap->HasChildren() == false && thread_heap->IsAddressInHeap(address) == true) {
    //            thread->SetLookupHeap(thread_heap);
    //            return thread_heap;
    //        }
    //    }
    //
    //    std::scoped_lock l(sHeapManagerMutex);
    //
    //    /* Lookup all children in thread's lookup heap */
    //    if (last_lookup_heap != nullptr && last_lookup_heap->HasChildren() == true) {
    //        mem::Heap contained_heap = last_lookup_heap->FindContainedHeap(address);
    //        if (contained_heap != nullptr) { thread->SetLookupHeap(contained_heap); return contained_heap; }
    //    }
    //
    //    /* Lookup all children in thread's current heap */
    //    if (thread_heap != nullptr && thread_heap->HasChildren() == true) {
    //        mem::Heap contained_heap = thread_heap->FindContainedHeap(address);
    //        if (contained_heap != nullptr) { thread->SetLookupHeap(contained_heap); return contained_heap; }
    //    }
    //
    //    /* If thread heaps fail fallback to the root heap */
    //    mem::Heap contained_heap = sRootHeap->FindContainedHeap(address)
    //    if (thread != nullptr && contained_heap != nullptr) { thread->SetLookupHeap(contained_heap); }
    //
    //    return contained_heap;
    //}
    //
    //namespace {
    //    Heap *FindHeapByNameImpl(Heap *parent_heap, const char *heap_name) {
    //        /* Sift children recursively */
    //        for (Heap &heap : parent_heap->m_child_list) {
    //
    //            /* Return heap on success */
    //            int result = ::strcmp(heap.GetName(), heap_name);
    //            if (result == 0) { return std::addressof(heap); }
    //
    //            /* Recurse through childs children on failure */
    //            if (heap.m_child_list.IsEmpty() == false) {
    //                Heap *candidate = FindHeapByName(std::addressof(heap), heap_name);
    //                if (candidate != nullptr) { return candidate; }
    //            }
    //        }
    //        return nullptr;
    //    }
    //}
    //
    //Heap *FindHeapByName(const char *heap_name) {
    //    std::scoped_lock l(sHeapManagerMutex);
    //    return FindHeapByNameImpl(sRootHeap, heap_name);
    //}
    //
    //Heap *GetCurrentThreadHeap() {
    //    sys::Thread *thread = sys::impl::ThreadManager::GetInstance()->GetCurrentThread();
    //    if (thread != nullptr) {
    //        return thread->GetThreadHeap();
    //    }
    //    return sRootHeap;
    //}

    bool IsHeapManagerInitialized() { return sIsHeapManagerInitialized; }

    ALWAYS_INLINE HeapManager *GetHeapManager() { return util::GetPointer(sHeapManagerStorage); }

    constexpr ALWAYS_INLINE sys::Mutex *GetHeapManagerLock() { return std::addressof(sHeapManagerMutex); }

    //void SetCurrentThreadHeap(Heap *heap) {
    //    sys::ThreadBase *thread = sys::impl::ThreadManager::GetInstance()->GetCurrentThread();
    //    if (thread != nullptr) {
    //        thread->SetThreadCurrentHeap(heap);
    //    }
    //}
    //
    //bool IsAddressFromAnyHeap(void *address) {
    //    return sRootHeap->IsAddressInHeap(address);
    //}
}
