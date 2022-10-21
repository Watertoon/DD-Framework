#pragma once

/* While the initial idea of some elements were inspired by Atmosphère, for this implementation I only used cppreference and vague mental notes from months ago */
/* If you can make a case for infringement please let me know and I'll correct it */

namespace dd::util {

    class IntrusiveListNode {
        public:
            IntrusiveListNode *m_prev;
            IntrusiveListNode *m_next;
        public:
            constexpr IntrusiveListNode() : m_prev(this), m_next(this) {/*...*/}

            constexpr IntrusiveListNode *next() const {
                return m_next;
            }

            constexpr IntrusiveListNode *prev() const {
                return m_prev;
            }
            
            
            constexpr bool IsLinked() const {
                return m_prev != m_next;
            }

            constexpr void LinkNext(IntrusiveListNode *new_node) {
                IntrusiveListNode *new_prev = new_node->m_prev;
                new_node->m_prev = m_prev;
                new_prev->m_next = this;
                m_prev->m_next = new_node;
                m_prev = new_prev;
            }

            constexpr void LinkPrev(IntrusiveListNode *new_node) {
                IntrusiveListNode *new_prev = new_node->m_prev;
                new_node->m_prev = m_prev->m_prev;
                new_prev->m_next = m_prev;
                m_prev->m_prev->m_next = new_node;
                m_prev->m_prev = new_prev;
            }

            constexpr void Unlink() {
                m_prev->m_next = m_next;
                m_next->m_prev = m_prev;
                m_next = this;
                m_prev = this;
            }
    };

    template<typename T, class Traits>
    class IntrusiveList {
        public:
            template <bool IsConst>
            class Iterator {
                public:
                    using value_type      = T;
                    using reference       = T&;
                    using const_reference = const T&;
                    using pointer         = T*;
                    using const_pointer   = const T*;
                private:
                    IntrusiveListNode *m_node;
                    IntrusiveListNode *m_next;
                public:
                    constexpr Iterator(IntrusiveListNode *node) : m_node(node), m_next(node->next()) {}

                    reference operator*() {
                        return Traits::GetParentReference(m_node);
                    }
                    const_reference operator*() const {
                        return Traits::GetParentReference(m_node);
                    }

                    constexpr bool operator!=(const Iterator<IsConst> &rhs) {
                        return m_node != rhs.m_node;
                    }

                    constexpr Iterator<IsConst> &operator++() {
                        m_node = m_next;
                        m_next = m_next->next();
                        return *this;
                    }
            };
        public:
            using value_type      = T;
            using reference       = T&;
            using const_reference = const T&;
            using pointer         = T*;
            using const_pointer   = const T*;
            using iterator        = Iterator<false>;
            using const_iterator  = Iterator<true>;
        private:
            IntrusiveListNode m_list;
        public:
            constexpr IntrusiveList() : m_list() {}

            constexpr iterator begin() {
                return iterator(m_list.m_next);
            }
            constexpr const_iterator cbegin() const {
                return const_iterator(m_list.m_next);
            }

            constexpr iterator end() {
                return iterator(std::addressof(m_list));
            }
            constexpr const_iterator cend() const {
                return const_iterator(std::addressof(m_list));
            }

            ALWAYS_INLINE reference Front() {
                return Traits::GetParentReference(m_list.m_next);
            }
            ALWAYS_INLINE const_reference Front() const {
                return Traits::GetParentReference(m_list.m_next);
            }

            ALWAYS_INLINE reference Back() {
                return Traits::GetParentReference(m_list.m_prev);
            }
            ALWAYS_INLINE const_reference Back() const {
                return Traits::GetParentReference(m_list.m_prev);
            }

            constexpr bool IsEmpty() const {
                return m_list.IsLinked();
            }

            void ALWAYS_INLINE PushBack(reference obj) {
                m_list.LinkNext(Traits::GetListNode(std::addressof(obj)));
            }

            void ALWAYS_INLINE PushFront(reference obj) {
                m_list.m_next->LinkNext(Traits::GetListNode(std::addressof(obj)));
            }
            
            constexpr reference PopFront() {
                IntrusiveListNode *front = m_list.m_next;
                front->Unlink();
                return Traits::GetParentReference(front);
            }

            constexpr const_reference PopFront() {
                IntrusiveListNode *front = m_list.m_next;
                front->Unlink();
                return Traits::GetParentReference(front);
            }

            constexpr reference PopBack() {
                IntrusiveListNode *back = m_list.m_prev;
                back->Unlink();
                return Traits::GetParentReference(back);
            }

            constexpr const_reference PopBack() {
                IntrusiveListNode *back = m_list.m_prev;
                back->Unlink();
                return Traits::GetParentReference(back);
            }

            static ALWAYS_INLINE void Remove(reference obj) {
                Traits::GetListNode(std::addressof(obj)).Unlink();
            }

            static constexpr iterator IteratorTo(reference obj) {
                return iterator(Traits::GetListNode(std::addressof(obj)));
            }

            static constexpr const_iterator IteratorTo(const_reference obj) {
                return const_iterator(Traits::GetListNode(std::addressof(obj)));
            }
    };

    template<class RP, auto M>
    struct IntrusiveListMemberTraits {

        static ALWAYS_INLINE RP *GetParent(IntrusiveListNode *node) {
            return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }
        static ALWAYS_INLINE RP &GetParentReference(IntrusiveListNode *node) {
            return *reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }

        static ALWAYS_INLINE const RP *GetParent(const IntrusiveListNode *node) {
            return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }
        static ALWAYS_INLINE const RP &GetParentReference(const IntrusiveListNode *node) {
            return *reinterpret_cast<const RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }

        static ALWAYS_INLINE IntrusiveListNode *GetListNode(const RP *parent) {
            return reinterpret_cast<IntrusiveListNode*>(reinterpret_cast<uintptr_t>(parent) + OffsetOf(M));
        }
    };

    template<class RP, auto M>
    struct IntrusiveListTraits {
        using Traits = IntrusiveListMemberTraits<RP, M>;
        using List   = IntrusiveList<ParentType<M>, Traits>;
    };
}
