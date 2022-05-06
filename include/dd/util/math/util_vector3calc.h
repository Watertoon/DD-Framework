#pragma once

namespace dd::util::math {
    
    /* Rotates a vector by "angle" along the x-axis */
    void RotateVectorAxisX(Vector3f *out_vector, const Vector3f& rot_vector, float angle);

    /* Rotates a vector by "angle" along the y-axis */
    void RotateVectorAxisY(Vector3f *out_vector, const Vector3f& rot_vector, float angle);

    /* Rotates a vector by "angle" along the y-axis */
    void RotateVectorAxisZ(Vector3f *out_vector, const Vector3f& rot_vector, float angle);

    /* Makes the vector "to_parallelize" parallel to "base"  */
    void Parallelize(Vector3f *out_vector, const Vector3f& base, const Vector3f& to_parallelize);
}