#pragma once

/* For implementation files */
#define DD_SINGLETON_TRAITS_IMPL(class_name) \
    constinit class_name *class_name::Instance = nullptr; \
    constinit class_name::SingletonDisposer *class_name::Disposer = nullptr

/* For class definitions */
#define DD_SINGLETON_TRAITS(class_name) \
    public: \
        class SingletonDisposer; \
    private: \
        static class_name *Instance; \
        static SingletonDisposer *Disposer; \
    public: \
        class SingletonDisposer : public dd::mem::IDisposer { \
            public: \
                SingletonDisposer(dd::mem::Heap *heap) : IDisposer(heap) { \
                    Disposer = this; \
                } \
                ~SingletonDisposer() { \
                    if (Disposer == this) { \
                        Disposer = nullptr; \
                        std::destroy_at(Instance); \
                        Instance = nullptr; \
                    } \
                } \
        }; \
    private: \
        dd::util::TypeStorage<SingletonDisposer> m_singleton_disposer; \
    public: \
        static ALWAYS_INLINE class_name *CreateInstance(dd::mem::Heap *allocate_heap) { \
            if (Instance != nullptr) { \
                return Instance; \
            } \
            Instance = reinterpret_cast<class_name*>(::operator new(sizeof(class_name), allocate_heap, alignof(class_name))); \
            std::construct_at(Instance); \
            dd::util::ConstructAt(Instance->m_singleton_disposer, allocate_heap); \
            return Instance; \
        } \
        static ALWAYS_INLINE void DestroyInstance() { \
            SingletonDisposer *disposer = Disposer; \
            if (Disposer == nullptr) { \
                return; \
            } \
            Disposer = nullptr; \
            std::destroy_at(disposer); \
            if (Instance != nullptr) { \
                std::destroy_at(Instance); \
                ::operator delete(Instance); \
            } \
            Instance = nullptr; \
        } \
        constexpr static ALWAYS_INLINE class_name *GetInstance() { return (std::is_constant_evaluated() == true) ? nullptr : Instance; }
