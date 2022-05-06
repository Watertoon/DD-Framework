#pragma once

/* While the initial idea came from Atmosphere for this implementation I only used cppreference with vague mental notes */
/* If you can make a case for infringement please let me know and I'll fix it */

namespace dd::util {

    template<typename T, size_t S = sizeof(T), size_t A = alignof(T)>
    struct TypeStorage {
        using UnderlyingType = T;
        std::aligned_storage<S, A>::type _storage;
    };

    template<typename T>
    T::UnderlyingType &GetReference(T& storage) {
        return *reinterpret_cast<T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T>
    const T::UnderlyingType &GetReference(const T& storage) {
        return *reinterpret_cast<const T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T>
    T::UnderlyingType *GetPointer(T& storage) {
        return reinterpret_cast<T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T>
    const T::UnderlyingType *GetPointer(const T& storage) {
        return reinterpret_cast<const T::UnderlyingType*>(std::addressof(storage._storage));
    }

    template<typename T, class... Args>
    void ConstructAt(T &storage, Args... args) {
        std::construct_at(GetPointer(storage), std::forward<Args>(args)...);
    }

    template<typename T>
    void ConstructAt(T &storage) {
        std::construct_at(GetPointer(storage));
    }

    template<typename T, class... Args>
    void DestructAt(T &storage, Args... args) {
        std::destroy_at(GetPointer(storage), std::forward<Args>(args)...);
    }

    template<typename T>
    void DestructAt(T &storage) {
        std::destroy_at(GetPointer(storage));
    }
}
