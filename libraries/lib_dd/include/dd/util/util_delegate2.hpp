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

    template<typename Param1, typename Param2>
    class IDelegate2 {
        public:

            virtual void Invoke(Param1 param1, Param2 param2) = 0;

            virtual IDelegate2 *Clone() const { return nullptr; }
    };

    template<typename T, typename Param1, typename Param2>
    class Delegate2 final : public IDelegate2<Param1, Param2> {
        public:
            using FunctionType = void (T::*)(Param1, Param2);
        private:
            T                *m_t;
            FunctionType      m_function;
        public:
            constexpr ALWAYS_INLINE explicit Delegate2(T *t, FunctionType function) : m_t(t), m_function(function) {/*...*/}

            virtual void Invoke(Param1 param1, Param2 param2) override final {
                (m_t->*m_function)(param1, param2);
            }

            virtual IDelegate2<Param1, Param2> *Clone() const override final {
                Delegate2<T, Param1, Param2> *new_delegate = new Delegate2<T, Param1, Param2>(m_t, m_function);
                return new_delegate; 
            }
    };
}
