#pragma once

/* For implementation files */
#define DD_SINGLETON_TRAITS_IMPL(class_name) \
    constinit class_name *class_name::sInstance = nullptr; \
    constinit class_name::SingletonDisposer *class_name::sDisposer = nullptr

/* For class definitions */
#define DD_SINGLETON_TRAITS(class_name) \
    public: \
        class SingletonDisposer; \
    private: \
        static class_name *sInstance; \
        static SingletonDisposer *sDisposer; \
    public: \
        class SingletonDisposer : public dd::mem::IDisposer { \
            public: \
                SingletonDisposer(dd::mem::Heap *heap) : IDisposer(heap) { \
                    sDisposer = this; \
                } \
                ~SingletonDisposer() { \
                    if (sDisposer == this) { \
                        sDisposer = nullptr; \
                        std::destroy_at(sInstance); \
                        sInstance = nullptr; \
                    } \
                } \
        }; \
    private: \
        dd::util::TypeStorage<SingletonDisposer> m_singleton_disposer; \
    public: \
        static ALWAYS_INLINE class_name *CreateInstance(dd::mem::Heap *allocate_heap) { \
            if (sInstance != nullptr) { \
                return sInstance; \
            } \
            sInstance = reinterpret_cast<class_name*>(::operator new(sizeof(class_name), allocate_heap, alignof(class_name))); \
            std::construct_at(sInstance); \
            dd::util::ConstructAt(sInstance->m_singleton_disposer, allocate_heap); \
            return sInstance; \
        } \
        static ALWAYS_INLINE void FinalizeInstance() { \
            SingletonDisposer *disposer = sDisposer; \
            if (sDisposer == nullptr) { \
                return; \
            } \
            sDisposer = nullptr; \
            std::destroy_at(disposer); \
            if (sInstance != nullptr) { \
                std::destroy_at(sInstance); \
                ::operator delete(sInstance); \
            } \
            sInstance = nullptr; \
        } \
        constexpr static ALWAYS_INLINE class_name *GetInstance() { return (std::is_constant_evaluated() == true) ? nullptr : sInstance; }
