#include "abcrUtils.h"

// c# strings are 2 bytes
void copyCharsWithStride(void* target, const string& source, size_t maxLength)
{
    const auto span = std::min(maxLength, source.size());
    for (size_t i = 0; i < span; ++i)
    {
        *(static_cast<char*>(target) + 2 * i + 0) = source[i];
        *(static_cast<char*>(target) + 2 * i + 1) = '\0';
    }

    // null ends
    *(static_cast<char*>(target) + 2 * span + 0) = '\0';
    *(static_cast<char*>(target) + 2 * span + 1) = '\0';
}

void computeMeshTangent(const V3f& p, const N3f& n, const V2f& uv, float* t)
{

}

void decomposeMatrix(const Imath::M44d& m, Imath::V3d& s, Imath::V3d& sh, Imath::Quatd& r, Imath::V3d& t)
{
    Imath::M44d mat(m);

    Imath::extractAndRemoveScalingAndShear(mat, s, sh, false);

    t.x = mat[3][0];
    t.y = mat[3][1];
    t.z = mat[3][2];

    r = Imath::extractQuat(mat);
}