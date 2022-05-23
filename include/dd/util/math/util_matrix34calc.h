#pragma once

namespace dd::util::math {

    /* WARNING: Avoid continous rotations with these unless you know what you're doing */
    NO_CONSTANT_PROPAGATION void RotateLocalX(Matrix34f *out_rot_matrix, float theta);

    NO_CONSTANT_PROPAGATION void RotateLocalY(Matrix34f *out_rot_matrix, float theta);

    NO_CONSTANT_PROPAGATION void RotateLocalZ(Matrix34f *out_rot_matrix, float theta);
}
