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

    template<typename Param>
    class IDelegate1 {
        public:

            virtual void Invoke(Param param) = 0;

            virtual IDelegate1 *Clone() const { return nullptr; }
    };

    template<typename T, typename Param>
    class Delegate1 final : public IDelegate1<Param> {
        public:
            using FunctionType = void (T::*)(Param);
        private:
            T                *m_t;
            FunctionType      m_function;
        public:
            constexpr ALWAYS_INLINE explicit Delegate1(T *t, FunctionType function) : m_t(t), m_function(function) {/*...*/}

            virtual void Invoke(Param param) override final {
                (m_t->*m_function)(param);
            }

            virtual IDelegate1<Param> *Clone() const override final {
                Delegate1<T, Param> *new_delegate = new Delegate1<T, Param>(m_t, m_function);
                return new_delegate; 
            }
    };
}
