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

    class Camera {
        private:
            math::Matrix34f m_camera_mtx;
        public:
            DD_RTTI_BASE(Camera);
        public:
            constexpr Camera() {}

            void UpdateCameraMatrixSelf() {
                this->UpdateCameraMatrix(std::addressof(m_camera_mtx));
            }

            constexpr math::Matrix34f *GetCameraMatrix() {
                return std::addressof(m_camera_mtx);
            }

            constexpr const math::Matrix34f *GetCameraMatrix() const {
                return std::addressof(m_camera_mtx);
            }

            constexpr inline virtual void UpdateCameraMatrix(math::Matrix34f *out_view_matrix) const = 0;
    };

    class LookAtCamera : public Camera {
        private:
            math::Vector3d  m_pos;
            math::Vector3d  m_at;
            math::Vector3f  m_up;
        public:
            DD_RTTI_DERIVED(LookAtCamera, Camera);
        public:
            constexpr LookAtCamera(const math::Vector3f& pos, const math::Vector3f& at, const math::Vector3f& up) : m_pos(pos), m_at(at), m_up(up) {/*...*/}

            constexpr inline virtual void UpdateCameraMatrix(math::Matrix34f *out_view_matrix) const override {

                /* Calculate normalized direction */
                math::Vector3f dir = m_pos - m_at;
                const float dir_mag = dir.Magnitude();
                if (0.0 < dir_mag) {
                    const float dir_norm = 1.0 / dir_mag;
                    dir = dir * dir_norm;
                }

                /* Calculate normalized right */
                math::Vector3f right = m_up.Cross(dir);
                const float right_mag = right.Magnitude();
                if (0.0 < right_mag) {
                    const float right_norm = 1.0 / right_mag;
                    right = right * right_norm;
                    
                }

                /* Calculate normalized up */
                const math::Vector3f up = dir.Cross(right);

                /* Set matrix */
                out_view_matrix->m_arr2d[0][0] = right.m_vec[0];
                out_view_matrix->m_arr2d[0][1] = right.m_vec[1];
                out_view_matrix->m_arr2d[0][2] = right.m_vec[2];
                out_view_matrix->m_arr2d[1][0] = up.m_vec[0];
                out_view_matrix->m_arr2d[1][1] = up.m_vec[1];
                out_view_matrix->m_arr2d[1][2] = up.m_vec[2];
                out_view_matrix->m_arr2d[2][0] = dir.m_vec[0];
                out_view_matrix->m_arr2d[2][1] = dir.m_vec[1];
                out_view_matrix->m_arr2d[2][2] = dir.m_vec[2];

                /* Calculate position inline */
                out_view_matrix->m_arr2d[0][3] = -(right.m_vec[0] * m_pos.m_vec[0] + right.m_vec[1] * m_pos.m_vec[1] + right.m_vec[2] * m_pos.m_vec[2]);
                out_view_matrix->m_arr2d[1][3] = -(up.m_vec[0]    * m_pos.m_vec[0] + up.m_vec[1]    * m_pos.m_vec[1] + up.m_vec[2]    * m_pos.m_vec[2]);
                out_view_matrix->m_arr2d[2][3] = -(dir.m_vec[0]   * m_pos.m_vec[0] + dir.m_vec[1]   * m_pos.m_vec[1] + dir.m_vec[2]   * m_pos.m_vec[2]);
            }

            void SetPos(const math::Vector3f& new_pos) {
                m_pos = new_pos;
            }

            void SetAt(const math::Vector3f& new_at) {
                m_at = new_at;
            }

            void SetUp(const math::Vector3f& new_up) {
                m_up = new_up;
            }

            constexpr void GetPos(math::Vector3f *out_pos) const {
                *out_pos = m_pos;
            }

           constexpr void GetAt(math::Vector3f *out_at) const {
                *out_at = m_at;
            }

           constexpr void GetUp(math::Vector3f *out_up) const {
                *out_up = m_up;
            }

            constexpr void GetRightVectorByMatrix(math::Vector3f *out_right_vec) const {
                const math::Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_right_vec[0] = camera_mtx->m_arr2d[0][0];
                out_right_vec[1] = camera_mtx->m_arr2d[0][1];
                out_right_vec[2] = camera_mtx->m_arr2d[0][2];
            }

            constexpr void GetLookDirVectorByMatrix(math::Vector3f *out_look_dir_vec) const {
                const math::Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_look_dir_vec[0] = camera_mtx->m_arr2d[1][0];
                out_look_dir_vec[1] = camera_mtx->m_arr2d[1][1];
                out_look_dir_vec[2] = camera_mtx->m_arr2d[1][2];
            }

            constexpr void GetUpVectorByMatrix(math::Vector3f *out_up_vec) const {
                const math::Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_up_vec[0] = camera_mtx->m_arr2d[2][0];
                out_up_vec[1] = camera_mtx->m_arr2d[2][1];
                out_up_vec[2] = camera_mtx->m_arr2d[2][2];
            }

            constexpr void GetWorldPosByMatrix(math::Vector3f *out_world_pos_vec) const {
                const math::Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_world_pos_vec[0] = (-(camera_mtx->m_arr2d[0][0] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][0] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][0] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[1] = (-(camera_mtx->m_arr2d[0][1] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][1] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][1] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[2] = (-(camera_mtx->m_arr2d[0][2] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][2] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][2] * camera_mtx->m_arr2d[2][3]);
            }

            constexpr void CameraPositionToWorldPositionByMatrix(math::Vector3f *out_world_pos_vec, const math::Vector3f& camera_pos) const {
                const math::Matrix34f *camera_mtx = this->GetCameraMatrix();
                out_world_pos_vec[0] = camera_pos.m_vec[0] + camera_pos.m_vec[1] + camera_pos.m_vec[2] + (-(camera_mtx->m_arr2d[0][0] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][0] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][0] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[1] = camera_pos.m_vec[0] + camera_pos.m_vec[1] + camera_pos.m_vec[2] + (-(camera_mtx->m_arr2d[0][1] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][1] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][1] * camera_mtx->m_arr2d[2][3]);
                out_world_pos_vec[2] = camera_pos.m_vec[0] + camera_pos.m_vec[1] + camera_pos.m_vec[2] + (-(camera_mtx->m_arr2d[0][2] * camera_mtx->m_arr2d[0][3]) - (camera_mtx->m_arr2d[1][2] * camera_mtx->m_arr2d[1][3])) - (camera_mtx->m_arr2d[2][2] * camera_mtx->m_arr2d[2][3]);
            }
    };
}
