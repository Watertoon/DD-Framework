#include <dd.hpp>

namespace dd::mem {
    
    IDisposer::IDisposer() {

        /* Find heap containing IDisposer */
        m_contained_heap = mem::FindContainedHeap(this);

        if (m_contained_heap != nullptr) {
            /* Add to contained heap */
            m_contained_heap->AppendDisposer(*this);
        }
    }

    IDisposer::IDisposer(Heap *heap) {
        
        /* Set contained heap */
        m_contained_heap = heap;
        
        /* Fallback to heap manager */
        if (m_contained_heap == nullptr) {
            m_contained_heap = mem::FindContainedHeap(this);
        }

        /* Add to contained heap */
        if (m_contained_heap != nullptr) {
            m_contained_heap->AppendDisposer(*this);
        }
    }

    IDisposer::~IDisposer() {
        
        /* Remove from contained heap disposer list */
        if (m_contained_heap != nullptr) {
            m_contained_heap->RemoveDisposer(*this);
            m_contained_heap = nullptr;
        }
    }
}
