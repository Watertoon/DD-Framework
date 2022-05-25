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

namespace dd::util::math {

    template<typename T>
        requires std::is_integral<T>::value || std::is_floating_point<T>::value
    class Vector3Type {
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 4))) v3;
            union {
                T m_vec[3];
                struct {
                    T x;
                    T y;
                    T z;
                };
            };
        public:
            constexpr ALWAYS_INLINE Vector3Type() : x(), y(), z() {/*...*/}
            constexpr ALWAYS_INLINE Vector3Type(T x, T y = 0, T z = 0) : x(x), y(y), z(z) {/*...*/}

            constexpr ALWAYS_INLINE Vector3Type(v3& copy) : x(copy[0]), y(copy[1]), z(copy[2]) {/*...*/}
            constexpr ALWAYS_INLINE Vector3Type(const v3& copy) : x(copy[0]), y(copy[1]), z(copy[2]) {/*...*/}
            constexpr ALWAYS_INLINE Vector3Type(const Vector3Type& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {/*...*/}

            template<typename A = T> requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == (sizeof(float) * 3))
            constexpr v3 GetVectorType() {
                return v3{x,y,z,0};
            }

            template<typename A = T> requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == (sizeof(float) * 3))
            constexpr const v3 GetVectorType() const {
                return v3{x,y,z,0};
            }

            template<typename A = T> requires std::is_integral<A>::value && std::is_signed<A>::value && (sizeof(Vector3Type<A>) == (sizeof(int) * 3))
            constexpr v3 GetVectorType() {
                return v3{x,y,z,0};
            }

            template<typename A = T> requires std::is_integral<A>::value && std::is_signed<A>::value && (sizeof(Vector3Type<A>) == (sizeof(int) * 3))
            constexpr const v3 GetVectorType() const {
                return v3{x,y,z,0};
            }

            template<typename A = T> requires std::is_integral<A>::value && std::is_unsigned<A>::value && (sizeof(Vector3Type<A>) == (sizeof(int) * 3))
            constexpr v3 GetVectorType() {
                return v3{x,y,z,0};
            }

            template<typename A = T> requires std::is_integral<A>::value && std::is_unsigned<A>::value && (sizeof(Vector3Type<A>) == (sizeof(int) * 3))
            constexpr const v3 GetVectorType() const {
                return v3{x,y,z,0};
            }

            constexpr Vector3Type& operator=(const Vector3Type& rhs) {
                x = rhs.x;
                y = rhs.y;
                z = rhs.z;
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type operator+(Vector3Type& rhs) {
                return Vector3Type(this->GetVectorType() + rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator+(const Vector3Type& rhs) const {
                return Vector3Type(this->GetVectorType() + rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator-(Vector3Type& rhs) {
                return Vector3Type(this->GetVectorType() - rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator-(const Vector3Type& rhs) const {
                return Vector3Type(this->GetVectorType() - rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator*(const T scalar) {
                return Vector3Type(this->x * scalar, this->y * scalar, this->z * scalar);
            }

            constexpr ALWAYS_INLINE Vector3Type operator*(const T scalar) const {
                return Vector3Type(this->x * scalar, this->y * scalar, this->z * scalar);
            }

            constexpr ALWAYS_INLINE Vector3Type& operator+=(Vector3Type& rhs) {
                const v3 v = this->GetVectorType() + rhs.GetVectorType();
                this->x = v[0];
                this->y = v[1];
                this->z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type& operator+=(const Vector3Type& rhs) {
                const v3 v = this->GetVectorType() + rhs.GetVectorType();
                this->x = v[0];
                this->y = v[1];
                this->z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type& operator-=(Vector3Type& rhs) {
                const v3 v = this->GetVectorType() - rhs.GetVectorType();
                this->x = v[0];
                this->y = v[1];
                this->z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type& operator-=(const Vector3Type& rhs) {
                const v3 v = this->GetVectorType() - rhs.GetVectorType();
                this->x = v[0];
                this->y = v[1];
                this->z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE bool operator==(Vector3Type& rhs) {
                return (x == rhs.x) & (y == rhs.y) & (z == rhs.z);
            }

            constexpr ALWAYS_INLINE bool operator==(const Vector3Type& rhs) {
                return (x == rhs.x) & (y == rhs.y) & (z == rhs.z);
            }

            constexpr ALWAYS_INLINE bool operator!=(Vector3Type& rhs) {
                return !(*this == rhs);
            }

            constexpr ALWAYS_INLINE bool operator!=(const Vector3Type& rhs) {
                return !(*this == rhs);
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) {
                const v3 a = sse4::shufps(this->GetVectorType(), this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v3 b = sse4::shufps(rhs.GetVectorType(), rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v3 c = sse4::mulps(a, rhs.GetVectorType());
                const v3 d = sse4::mulps(a, b);
                const v3 e = sse4::shufps(c, c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(sse4::subps(d, e));
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) const {
                const v3 a = sse4::shufps(this->GetVectorType(), this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v3 b = sse4::shufps(rhs.GetVectorType(), rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v3 c = sse4::mulps(a, rhs.GetVectorType());
                const v3 d = sse4::mulps(a, b);
                const v3 e = sse4::shufps(c, c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(sse4::subps(d, e));
            }

            template<typename A = T> requires std::is_integral<A>::value && (sizeof(Vector3Type<A>) == sizeof(s32) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) {
                const v3 a = sse4::pshufd(this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v3 b = sse4::pshufd(rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v3 c = sse4::pmuld(a, rhs.GetVectorType());
                const v3 d = sse4::pmuld(a, b);
                const v3 e = sse4::pshufd(c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(sse4::psubd(d, e));
            }

            template<typename A = T> requires std::is_integral<A>::value && (sizeof(Vector3Type<A>) == sizeof(s32) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) const {
                const v3 a = sse4::pshufd(this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v3 b = sse4::pshufd(rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v3 c = sse4::pmuld(a, rhs.GetVectorType());
                const v3 d = sse4::pmuld(a, b);
                const v3 e = sse4::pshufd(c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(sse4::psubd(d, e));
            }

            constexpr ALWAYS_INLINE T Dot(const Vector3Type& rhs) {
                const v3 temp = this->GetVectorType() * rhs.GetVectorType();
                return temp[0] + temp[1] + temp[2];
            }

            constexpr ALWAYS_INLINE const T Dot(const Vector3Type& rhs) const {
                const v3 temp = this->GetVectorType() * rhs.GetVectorType();
                return temp[0] + temp[1] + temp[2];
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE T Magnitude() {
                return ::sqrtf(m_vec[0] * m_vec[0] + m_vec[1] * m_vec[1] + m_vec[2] * m_vec[2]);
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE const T Magnitude() const {
                return ::sqrtf(m_vec[0] * m_vec[0] + m_vec[1] * m_vec[1] + m_vec[2] * m_vec[2]);
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type<A> Normalize() {
                Vector3Type<A> ret = *this;
                const float mag = this->Magnitude();
                if (0.0 < mag) {
                    const float norm = 1.0 / mag;
                    ret = ret * norm;
                }
                return ret;
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type<A> Normalize() const {
                Vector3Type<A> ret = *this;
                const float mag = this->Magnitude();
                if (0.0 < mag) {
                    const float norm = 1.0 / mag;
                    ret = ret * norm;
                }
                return ret;
            }
    };

    using Vector3f = Vector3Type<float>;
    using Vector3u  = Vector3Type<unsigned int>;
    using Vector3i  = Vector3Type<int>;
    using Vector3d = Vector3Type<double>;

    template<typename T>
    constexpr Vector3Type<T> ZeroVector3 = {};

    template<typename T>
    constexpr Vector3Type<T> ezVector3(0, 0, 1);

    static_assert((Vector3u(sse4::v4ui{2,2,2}) + Vector3u(sse4::v4ui{2,4,2})) != Vector3u(sse4::v4ui{2,2,2}));
    static_assert((Vector3f(sse4::v4s{1.0f,1.0f,1.0f}) + Vector3f(sse4::v4s{1.0f,1.0f,1.0f})) == Vector3f(sse4::v4s{2.0f,2.0f,2.0f}));
    static_assert(Vector3f(sse4::v4s{1.0f,0.0f,1.0f}).Dot(Vector3f(sse4::v4s{1.0f,1.0f,0.0f})) == 1.0f);
}
