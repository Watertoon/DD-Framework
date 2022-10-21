#pragma once

namespace dd::util {

    class RuntimeTypeInfo {
        public:
            const RuntimeTypeInfo *m_next;
        public:
            explicit constexpr RuntimeTypeInfo() : m_next(nullptr) {/*...*/}
            explicit RuntimeTypeInfo(const RuntimeTypeInfo *base_class) : m_next(base_class) {/*...*/}

            constexpr ALWAYS_INLINE bool IsSameType(const RuntimeTypeInfo *other_info) const { return this == other_info; }
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
                    if (target->IsSameType(class_info) == true) { return true; } \
                    class_info = class_info->m_next; \
                } while (class_info != 0); \
                return false; \
            }
}
