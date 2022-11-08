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

    class Projection {
        protected:
            math::Matrix44f m_projection_matrix;
            bool            m_need_update;
        public:
            constexpr Projection() : m_need_update(true) {/*...*/}

            constexpr void UpdateProjectionMatrixSelf() {
                this->UpdateMatrix(std::addressof(m_projection_matrix));
                m_need_update = false;
            }

            constexpr math::Matrix44f *GetProjectionMatrix() {
                if (m_need_update == true) {
                    this->UpdateProjectionMatrixSelf();
                }
                return std::addressof(m_projection_matrix);
            }

            constexpr const math::Matrix44f *GetProjectionMatrix() const {
                return std::addressof(m_projection_matrix);
            }

            constexpr virtual void UpdateMatrix(math::Matrix44f *out_proj_matrix) const = 0;
    };
    
    class FrustumProjection : public Projection {
        public:
            friend class PerspectiveProjection;
        protected:
            float m_near;
            float m_far;
            float m_top;
            float m_bottom;
            float m_left;
            float m_right;
        public:
            constexpr FrustumProjection() {/*...*/}
            constexpr FrustumProjection(float near, float far, const BoundingBox2<float>& bound_box) : Projection(), m_near(near), m_far(far), m_top(bound_box.max_y), m_bottom(bound_box.min_y), m_left(bound_box.max_x), m_right(bound_box.min_x) {/*...*/}
            constexpr FrustumProjection(float near, float far, float top, float bottom, float left, float right) : Projection(), m_near(near), m_far(far), m_top(top), m_bottom(bottom), m_left(left), m_right(right) {/*...*/}

            constexpr virtual void UpdateMatrix(math::Matrix44f *out_proj_matrix) const override {
                const float reciprocal_a = 1.0f / (m_right + m_left);
                const math::Vector4f row1 = {
                    (m_near + m_near) *  reciprocal_a,
                    0.0f,
                    (m_left + m_right) * reciprocal_a,
                    0.0f
                };
                out_proj_matrix->m_row1 = row1;

                const float reciprocal_b = 1.0f / (m_top - m_bottom);
                const math::Vector4f row2 = {
                    0.0f,
                    (m_near + m_near) * reciprocal_b,
                    (m_top + m_bottom) * reciprocal_b,
                    0.0f
                };
                out_proj_matrix->m_row2 = row2;

                const float reciprocal_c = 1.0f / (m_far - m_near);
                const math::Vector4f row3 = {
                    0.0f,
                    0.0f,
                    -((m_near + m_far) * reciprocal_c),
                    -((m_far + m_far) * m_near * reciprocal_c)
                };
                out_proj_matrix->m_row3 = row3;

                out_proj_matrix->m_row4 = { 0.0f, 0.0f, -1.0f, 0.0f };
            }
    };

    class PerspectiveProjection : public FrustumProjection {
        private:
            float m_fov_x;
        public:
            constexpr PerspectiveProjection() : FrustumProjection(1.0f, 10000.0f, math::TRadians<float, 45.0f>, ::sinf(math::TRadians<float, 45.0f> / 2), ::cosf(math::TRadians<float, 45.0f> / 2), ::tanf(math::TRadians<float, 45.0f> / 2)), m_fov_x(4.0 / 3.0f) {/*...*/}
            /* For some reason GCC seems to act very strangely with the first 2 parameters */
            constexpr PerspectiveProjection(float near_1, float far_2, float fovy, float aspect) : FrustumProjection(near_1, far_2, fovy, ::sinf(fovy / 2), ::cosf(fovy / 2), ::tanf(fovy / 2)), m_fov_x(aspect) { m_near = near_1; m_far = far_2; }

            constexpr virtual void UpdateMatrix(math::Matrix44f *out_proj_matrix) const override {
                const math::Vector4f row1 = {
                    1.0f / (m_right * m_fov_x),
                    0.0f,
                    0.0f,
                    0.0f
                };
                out_proj_matrix->m_row1 = row1;

                const math::Vector4f row2 = {
                    0.0f,
                    1.0f / m_right,
                    0.0f,
                    0.0f
                };
                out_proj_matrix->m_row2 = row2;

                const float reciprocal_c = 1.0f / (m_far - m_near);
                const math::Vector4f row3 = {
                    0.0f,
                    0.0f,
                    -((m_near + m_far) * reciprocal_c),
                    -((m_far + m_far) * m_near * reciprocal_c)
                };
                out_proj_matrix->m_row3 = row3;

                out_proj_matrix->m_row4 = { 0.0f, 0.0f, -1.0f, 0.0f };
            }

            void SetAspect(float new_aspect) {
                m_fov_x = new_aspect;
                m_need_update = true;
            }

            void SetFovX(float new_fov_x) {
                m_fov_x = ::tanf(new_fov_x * 0.5f) / m_right;
                m_need_update = true;
            }
    };
}
