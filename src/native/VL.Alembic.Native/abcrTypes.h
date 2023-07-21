#pragma once
// Copyright (c) .NET Foundation and Contributors (https://dotnetfoundation.org/ & https://stride3d.net) and Silicon Studio Corp. (https://www.siliconstudio.co.jp)
// Distributed under the MIT license. See the LICENSE.md file in the project root for more information.
//
// -----------------------------------------------------------------------------
// Original code from SlimMath project. http://code.google.com/p/slimmath/
// Greetings to SlimDX Group. Original code published with the following license:
// -----------------------------------------------------------------------------
/*
* Copyright (c) 2007-2011 SlimDX Group
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include <Alembic\Abc\All.h>

using namespace Alembic::Abc;

struct Vector2
{
	float x, y;

	Vector2() { x = y = 0; };
	Vector2(float x, float y) : x(x), y(y) {};
};

struct Vector3
{
	float x, y, z;

	Vector3() { x = 0, y = 0, z = 0; }
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
};

struct Vector4
{
	float x, y, z, w;

	Vector4() { x = y = z = w = 0; }
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Quaternion
{
	float x, y, z, w;
	Quaternion() { x = y = z = w = 0; }
	Quaternion(float x, float y, float z, float w) :
		x(x), y(y), z(z), w(w) {};
};

struct Matrix4x4
{
	float m11, m21, m31, m41,
		m12, m22, m32, m42,
		m13, m23, m33, m43,
		m14, m24, m34, m44;

	Matrix4x4() { m11 = m12 = m13 = m14 = m21 = m22 = m23 = m24 = m31 = m32 = m33 = m34 = m41 = m42 = m43 = m44 = 0; }

	static Matrix4x4 Identity()
	{
		Matrix4x4 result;
		result.m11 = 1.0f;
		result.m22 = 1.0f;
		result.m33 = 1.0f;
		result.m44 = 1.0f;

		return result;
	}

	static Matrix4x4 RotationQuaternion(const Quaternion& quaternion)
	{
		Matrix4x4 result = Matrix4x4::Identity();

		float xx = quaternion.x * quaternion.x;
		float yy = quaternion.y * quaternion.y;
		float zz = quaternion.z * quaternion.z;
		float xy = quaternion.x * quaternion.y;
		float zw = quaternion.z * quaternion.w;
		float zx = quaternion.z * quaternion.x;
		float yw = quaternion.y * quaternion.w;
		float yz = quaternion.y * quaternion.z;
		float xw = quaternion.x * quaternion.w;
		result.m11 = 1.0f - (2.0f * (yy + zz));
		result.m12 = 2.0f * (xy + zw);
		result.m13 = 2.0f * (zx - yw);
		result.m21 = 2.0f * (xy - zw);
		result.m22 = 1.0f - (2.0f * (zz + xx));
		result.m23 = 2.0f * (yz + xw);
		result.m31 = 2.0f * (zx + yw);
		result.m32 = 2.0f * (yz - xw);
		result.m33 = 1.0f - (2.0f * (yy + xx));

		return result;
	}

	Matrix4x4 Invert()
	{
		float b0 = (this->m31 * this->m42) - (this->m32 * this->m41);
		float b1 = (this->m31 * this->m43) - (this->m33 * this->m41);
		float b2 = (this->m34 * this->m41) - (this->m31 * this->m44);
		float b3 = (this->m32 * this->m43) - (this->m33 * this->m42);
		float b4 = (this->m34 * this->m42) - (this->m32 * this->m44);
		float b5 = (this->m33 * this->m44) - (this->m34 * this->m43);

		float d11 = this->m22 * b5 + this->m23 * b4 + this->m24 * b3;
		float d12 = this->m21 * b5 + this->m23 * b2 + this->m24 * b1;
		float d13 = this->m21 * -b4 + this->m22 * b2 + this->m24 * b0;
		float d14 = this->m21 * b3 + this->m22 * -b1 + this->m23 * b0;

		float det = this->m11 * d11 - this->m12 * d12 + this->m13 * d13 - this->m14 * d14;
		if (abs(det) == 0.0f)
			return Matrix4x4();

		det = 1.0f / det;

		float a0 = (this->m11 * this->m22) - (this->m12 * this->m21);
		float a1 = (this->m11 * this->m23) - (this->m13 * this->m21);
		float a2 = (this->m14 * this->m21) - (this->m11 * this->m24);
		float a3 = (this->m12 * this->m23) - (this->m13 * this->m22);
		float a4 = (this->m14 * this->m22) - (this->m12 * this->m24);
		float a5 = (this->m13 * this->m24) - (this->m14 * this->m23);

		float d21 = this->m12 * b5 + this->m13 * b4 + this->m14 * b3;
		float d22 = this->m11 * b5 + this->m13 * b2 + this->m14 * b1;
		float d23 = this->m11 * -b4 + this->m12 * b2 + this->m14 * b0;
		float d24 = this->m11 * b3 + this->m12 * -b1 + this->m13 * b0;

		float d31 = this->m42 * a5 + this->m43 * a4 + this->m44 * a3;
		float d32 = this->m41 * a5 + this->m43 * a2 + this->m44 * a1;
		float d33 = this->m41 * -a4 + this->m42 * a2 + this->m44 * a0;
		float d34 = this->m41 * a3 + this->m42 * -a1 + this->m43 * a0;

		float d41 = this->m32 * a5 + this->m33 * a4 + this->m34 * a3;
		float d42 = this->m31 * a5 + this->m33 * a2 + this->m34 * a1;
		float d43 = this->m31 * -a4 + this->m32 * a2 + this->m34 * a0;
		float d44 = this->m31 * a3 + this->m32 * -a1 + this->m33 * a0;

		Matrix4x4 result;
		result.m11 = +d11 * det; result.m12 = -d21 * det; result.m13 = +d31 * det; result.m14 = -d41 * det;
		result.m21 = -d12 * det; result.m22 = +d22 * det; result.m23 = -d32 * det; result.m24 = +d42 * det;
		result.m31 = +d13 * det; result.m32 = -d23 * det; result.m33 = +d33 * det; result.m34 = -d43 * det;
		result.m41 = -d14 * det; result.m42 = +d24 * det; result.m43 = -d34 * det; result.m44 = +d44 * det;

		return result;
	}
};

struct BoundingBox
{
	Vector3 minimum;
	Vector3 maximum;

	BoundingBox() { minimum = maximum = Vector3(); };
	BoundingBox(const Vector3& min, const Vector3& max)
		: minimum(min), maximum(max) {};
};

struct CameraParam
{
	float Aperture;
	float Near;
	float Far;
	float FocalLength;
	float FoV;

	CameraParam() { Aperture = Near = Far = FocalLength = FoV = 0; };
	CameraParam(float aperture, float near, float far, float focal, float fov) :
		Aperture(aperture), Near(near), Far(far), FocalLength(focal), FoV(fov) {};
};

struct DataPointer
{
	void* Pointer;
	int Size;

	DataPointer(void* ptr, int size) : Pointer(ptr), Size(size) {}
};
