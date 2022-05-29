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

namespace dd::util {

    class IDelegate {
        public:

            virtual void Invoke() = 0;

            virtual IDelegate *Clone() const { return nullptr; }
    };

    template<typename T>
    class Delegate final : public IDelegate {
        public:
            using FunctionType = void (T::*)();
        private:
            T                *m_t;
            FunctionType      m_function;
        public:
            constexpr ALWAYS_INLINE explicit Delegate(T *t, FunctionType function) : m_t(t), m_function(function) {/*...*/}

            virtual void Invoke() override final {
                (m_t->*m_function)();
            }

            virtual IDelegate *Clone() const override final {
                Delegate<T> *new_delegate = new Delegate<T>(m_t, m_function);
                return new_delegate; 
            }
    };
}
