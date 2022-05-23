#include <dd.hpp>

namespace dd::util::math {
    
    void RotateLocalX(Matrix34f *out_rot_matrix, float theta) {
        const float sin = SampleSin(theta * (static_cast<float>(AngleIndexHalfRound) / FloatPi));
        const float cos = SampleCos(theta * (static_cast<float>(AngleIndexHalfRound) / FloatPi));
        const float m12 = out_rot_matrix->m_arr2d[0][1];
        const float m22 = out_rot_matrix->m_arr2d[1][1];
        const float m32 = out_rot_matrix->m_arr2d[2][1];
        out_rot_matrix->m_arr2d[0][1] = (m12 * cos)                           + (out_rot_matrix->m_arr2d[0][2] * sin);
        out_rot_matrix->m_arr2d[0][2] = (out_rot_matrix->m_arr2d[0][2] * cos) - (m12 * sin);
        out_rot_matrix->m_arr2d[1][1] = (m22 * cos)                           + (out_rot_matrix->m_arr2d[1][2] * sin);
        out_rot_matrix->m_arr2d[1][2] = (out_rot_matrix->m_arr2d[1][2] * cos) - (m22 * sin);
        out_rot_matrix->m_arr2d[2][1] = (m32 * cos)                           + (out_rot_matrix->m_arr2d[2][2] * sin);
        out_rot_matrix->m_arr2d[2][2] = (out_rot_matrix->m_arr2d[2][2] * cos) - (m32 * sin);
    }

    void RotateLocalY(Matrix34f *out_rot_matrix, float theta) {
        const float sin = SampleSin(theta * (static_cast<float>(AngleIndexHalfRound) / FloatPi));
        const float cos = SampleCos(theta * (static_cast<float>(AngleIndexHalfRound) / FloatPi));
        const float m11 = out_rot_matrix->m_arr2d[0][0];
        const float m21 = out_rot_matrix->m_arr2d[1][0];
        const float m31 = out_rot_matrix->m_arr2d[2][0];
        out_rot_matrix->m_arr2d[0][0] = (m11 * cos) - (out_rot_matrix->m_arr2d[0][2] * sin);
        out_rot_matrix->m_arr2d[0][2] = (out_rot_matrix->m_arr2d[0][2] * cos) + (m11 * sin);
        out_rot_matrix->m_arr2d[1][0] = (m21 * cos) - (out_rot_matrix->m_arr2d[1][2] * sin);
        out_rot_matrix->m_arr2d[1][2] = (out_rot_matrix->m_arr2d[1][2] * cos) + (m21 * sin);
        out_rot_matrix->m_arr2d[2][0] = (m31 * cos) - (out_rot_matrix->m_arr2d[2][2] * sin);
        out_rot_matrix->m_arr2d[2][2] = (out_rot_matrix->m_arr2d[2][2] * cos) + (m31 * sin);
    }

    void RotateLocalZ(Matrix34f *out_rot_matrix, float theta) {
        const float sin = SampleSin(theta * (static_cast<float>(AngleIndexHalfRound) / FloatPi));
        const float cos = SampleCos(theta * (static_cast<float>(AngleIndexHalfRound) / FloatPi));
        const float m11 = out_rot_matrix->m_arr2d[0][0];
        const float m21 = out_rot_matrix->m_arr2d[1][0];
        const float m31 = out_rot_matrix->m_arr2d[2][0];
        out_rot_matrix->m_arr2d[0][0] = (m11 * cos)                           + (out_rot_matrix->m_arr2d[0][1] * sin);
        out_rot_matrix->m_arr2d[0][1] = (out_rot_matrix->m_arr2d[0][1] * cos) - (m11 * sin);
        out_rot_matrix->m_arr2d[1][0] = (m21 * cos)                           + (out_rot_matrix->m_arr2d[1][1] * sin);
        out_rot_matrix->m_arr2d[1][1] = (out_rot_matrix->m_arr2d[1][1] * cos) - (m21 * sin);
        out_rot_matrix->m_arr2d[2][0] = (m31 * cos)                           + (out_rot_matrix->m_arr2d[2][1] * sin);
        out_rot_matrix->m_arr2d[2][1] = (out_rot_matrix->m_arr2d[2][1] * cos) - (m31 * sin);
    }
}
