#pragma once

namespace dd::util::math {

    /* WARNING: Avoid continous rotations with these unless you know what you're doing */
    void RotateLocalX(Matrix34f *out_rot_matrix, const float theta);

    void RotateLocalY(Matrix34f *out_rot_matrix, const float theta);

    void RotateLocalZ(Matrix34f *out_rot_matrix, const float theta);
}
