#pragma once

namespace dd::util {

    template<typename Param>
    class IDelegate1 {
        public:

            virtual void Invoke(Param *param) = 0;

            virtual IDelegate1 *Clone() const { return nullptr; }
    };

    template<typename T, typename Param>
    class Delegate1 : public IDelegate1 {
        public:
            using FunctionType = void ()(T *, Param *);
        private:
            T                *m_t;
            FunctionType      m_function;
        public:
            constexpr ALWAYS_INLINE Delegate1(T *t, FunctionType function) : m_t(t), m_function(m_function) {/*...*/}

            virtual void Invoke(Param *param) {
                (m_function)(m_t, param);
            }

            virtual IDelegate1 *Clone() const {
                Delegate1<T, Param> *new_delegate = new Delegate1<T, Param>(m_t, m_function);
                return new_delegate; 
            }
    }
}
