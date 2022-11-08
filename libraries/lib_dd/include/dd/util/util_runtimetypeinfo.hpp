 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace dd::util {

    class RuntimeTypeInfo {
        public:
            const RuntimeTypeInfo *m_next;
        public:
            explicit constexpr RuntimeTypeInfo() : m_next(nullptr) {/*...*/}
            explicit RuntimeTypeInfo(const RuntimeTypeInfo *base_class) : m_next(base_class) {/*...*/}
    };

    #define RTTI_BASE(class_name) \
        protected: \
            using RootType = class_name; \
            static constexpr const RuntimeTypeInfo RootRTTI = nullptr; \
            static constexpr ALWAYS_INLINE const RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(RootRTTI); \
            } \
        public: \
            virtual constexpr ALWAYS_INLINE const RuntimeTypeInfo *GetRuntimeTypeInfo() const { \
                return GetRuntimeTypeInfoStatic(); \
            } \

    #define RTTI_DERIVED(derived_class, base_class) \
        protected: \
            static constexpr const RuntimeTypeInfo DerivedRTTI = base_class::GetRuntimeTypeInfoStatic(); \
            static constexpr ALWAYS_INLINE const RuntimeTypeInfo *GetRuntimeTypeInfoStatic() { \
                return std::addressof(DerivedRTTI); \
            } \
        public: \
            virtual constexpr ALWAYS_INLINE const RuntimeTypeInfo *GetRuntimeTypeInfo() const override { \
                return GetRuntimeTypeInfoStatic(); \
            } \
                \
            static constexpr ALWAYS_INLINE bool CheckRuntimeTypeInfo(RootType *other_obj) { \
                const RuntimeTypeInfo *target     = other_obj->GetRuntimeTypeInfo(); \
                const RuntimeTypeInfo *class_info = GetRuntimeTypeInfoStatic(); \
                do { \
                    if (class_info != target) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != 0); \
                return class_info == target; \
            }
}
