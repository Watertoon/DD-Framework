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
#include <dd.hpp>

namespace dd::util::math {

    /* Rotates a vector by "angle" along the x-axis */
    void RotateVectorAxisX(Vector3f *out_vector, const Vector3f& rot_vector, float angle) {
        const float index = angle * (static_cast<float>(AngleIndexHalfRound) / FloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        /*  | x                       |
         *  | y*sin(ang) - z*cos(ang) |
         *  | y*cos(ang) + z*sin(ang) |
         */ 
        out_vector->m_vec[0] = rot_vector.m_vec[0];
        out_vector->m_vec[1] = (rot_vector.m_vec[1] * sin) - (cos * rot_vector.m_vec[2]);
        out_vector->m_vec[2] = (rot_vector.m_vec[1] * cos) + (sin * rot_vector.m_vec[2]);
    }

    /* Rotates a vector by "angle" along the y-axis */
    void RotateVectorAxisY(Vector3f *out_vector, const Vector3f& rot_vector, float angle) {
        const float index = angle * (static_cast<float>(AngleIndexHalfRound) / FloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        /*  | z*cos(ang) + x*sin(ang) |
         *  | y                       |
         *  | z*sin(ang) - x*cos(ang) |
         */ 
        out_vector->m_vec[0] = (rot_vector.m_vec[2] * cos) + (sin * rot_vector.m_vec[0]);
        out_vector->m_vec[1] = rot_vector.m_vec[1];
        out_vector->m_vec[2] = (rot_vector.m_vec[2] * sin) - (cos * rot_vector.m_vec[0]);
    }

    /* Rotates a vector by "angle" along the y-axis */
    void RotateVectorAxisZ(Vector3f *out_vector, const Vector3f& rot_vector, float angle) {
        const float index = angle * (static_cast<float>(AngleIndexHalfRound) / FloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        /*  | x*sin(ang) - y*cos(ang) |
         *  | x*cos(ang) + y*sin(ang) |
         *  | z                       |
         */ 
        out_vector->m_vec[1] = rot_vector.m_vec[1];
        out_vector->m_vec[2] = (rot_vector.m_vec[2] * cos) - (sin * rot_vector.m_vec[0]);
        out_vector->m_vec[0] = (rot_vector.m_vec[2] * sin) + (cos * rot_vector.m_vec[0]);
    }

    /* Makes the vector "to_parallelize" parallel to base  */
    void Parallelize(Vector3f *out_vector, const Vector3f& base, const Vector3f& to_parallelize) {
        const float dot = base.Dot(to_parallelize);
        out_vector->m_vec[0] = to_parallelize.m_vec[0] * dot;
        out_vector->m_vec[1] = to_parallelize.m_vec[1] * dot;
        out_vector->m_vec[2] = to_parallelize.m_vec[2] * dot;
    }
}
