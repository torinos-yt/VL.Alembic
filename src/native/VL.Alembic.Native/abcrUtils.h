#pragma once

#include <Alembic\Abc\All.h>

#include "abcrTypes.h"

using namespace std;

void copyCharsWithStride(void* target, const string& source, size_t maxLength);

void computeMeshTangent(const V3f& p, const N3f& n, const V2f& uv, float* t);

inline N3f computeFaceNormal(const V3f& v0, const V3f& v1, const V3f& v2)
{
    return ((v1 - v0).cross(v2 - v0)).normalize();
}

inline Vector2 toVVVV(const Alembic::Abc::V2f& v)
{
    return Vector2(v.x, 1 - v.y);
}

inline Vector3 toVVVV(const Alembic::Abc::V3f& v)
{
    return Vector3(v.x, v.y, v.z);
}

inline Vector3 toVVVV(const Alembic::Abc::V3d& v)
{
    return Vector3(v.x, v.y, v.z);
}

inline Vector4 toVVVV(const Alembic::Abc::C4f& v)
{
    return Vector4(v.r, v.g, v.b, v.a);
}

inline Vector4 toVVVV(const Alembic::Abc::C3f& v)
{
    return Vector4(v.x, v.y, v.z, 1.0f);
}

inline Quaternion toVVVV(const Imath::Quatd& q)
{
    return Quaternion(q.v[0], q.v[1], q.v[2], q.r);
}

inline Matrix4x4 toVVVV(const Imath::M44f& m)
{
    Matrix4x4 result;

    result.m11 = m[0][0];
    result.m21 = m[1][0];
    result.m31 = m[2][0];
    result.m41 = m[3][0];
    result.m12 = m[0][1];
    result.m22 = m[1][1];
    result.m32 = m[2][1];
    result.m42 = m[3][1];
    result.m13 = m[0][2];
    result.m23 = m[1][2];
    result.m33 = m[2][2];
    result.m43 = m[3][2];
    result.m14 = m[0][3];
    result.m24 = m[1][3];
    result.m34 = m[2][3];
    result.m44 = m[3][3];

    return result;
}

inline void copyTo(float*& dst, const V2f& v)
{
    *dst++ = v.x;
    *dst++ = 1 - v.y;
}

inline void copyTo(float*& dst, const V3f& v)
{
    *dst++ = v.x;
    *dst++ = v.y;
    *dst++ = v.z;
}

inline void copyTo(float*& dst, const C3f& v)
{
    *dst++ = v.x;
    *dst++ = v.y;
    *dst++ = v.z;
    *dst++ = 1.0f;
}

inline void copyTo(float*& dst, const C4f& v)
{
    *dst++ = v.r;
    *dst++ = v.g;
    *dst++ = v.b;
    *dst++ = v.a;
}
