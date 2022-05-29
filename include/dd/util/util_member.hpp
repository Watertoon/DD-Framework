 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

/* While the initial idea of some elements were inspired by Atmosphère, for this implementation I only used cppreference and vague mental notes from months ago */
/* If you can make a case for infringement please let me know and I'll correct it */

namespace dd::util {

    template<class T>
    using element_type = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

    template <typename T>
    struct Parent { static T a; };

    template <typename T>
    struct ParentOfMemberTraits;

    template <typename M, typename P>
    struct ParentOfMemberTraits<M P::*> {
        using Parent = P;
        using Member = M;
    };

    template <auto M>
    using ParentType = ParentOfMemberTraits<decltype(M)>::Parent;

    template <auto M>
    struct OffsetOfImpl {
        using Parent = ParentOfMemberTraits<decltype(M)>::Parent;
        using Member = ParentOfMemberTraits<decltype(M)>::Member;
        union PStorage {
            Parent parent;
            unsigned char c;

            constexpr  PStorage() : c() {}
            constexpr ~PStorage() {}
        };
        static constexpr PStorage d = {};

        static ptrdiff_t GetOffset () {
            return reinterpret_cast<const unsigned char *>(std::addressof(d.parent.*M)) - std::addressof(d.c);
        }

        static ptrdiff_t GetOffsetAfter () {
            return util::AlignUp(sizeof(Member) + (reinterpret_cast<const unsigned char *>(std::addressof(d.parent.*M)) - std::addressof(d.c)), alignof(Parent));
        }
    };

    template<auto Ptr>
    ptrdiff_t OffsetOf() {
        return OffsetOfImpl<Ptr>::GetOffset();
    }

    template<auto Ptr>
    ptrdiff_t OffsetOfElementAfter() {
        return OffsetOfImpl<Ptr>::GetOffsetAfter();
    }

    /* Only use on virtual functions */
    template <class T>
    int GetVTableOffset(T function) {
        union
        {
            T pointer;
            int i;
        };
        pointer = function;

        return i - 1;
    }
}
