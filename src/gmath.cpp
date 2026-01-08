#include "gmath.h"

#include <cassert>
#include <cmath>

Vec3 Vec3::cross(Vec3 other) const {
	const Vec3& a { *this };
	Vec3& b { other };
	
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

float Vec3::dot(Vec3 other) const {
	return x * other.x + y * other.y + z * other.z;
}

float Vec3::magnitude() const {
	return sqrt(x*x + y*y + z*z);
}

Vec3 Vec3::normalised() const {
	float m { magnitude() };
	return {
		x / m,
		y / m,
		z / m
	};
}

Vec3 Vec3::operator*(float scalar) const {
	return {
		x * scalar,
		y * scalar,
		z * scalar
	};
}

Vec3 Vec3::operator+(Vec3 other) const {
	return { x + other.x, y + other.y, z + other.z };
}

void Vec3::operator+=(Vec3 other) {
	x += other.x;
	y += other.y;
	z += other.z;
}

Vec3 Vec3::operator-() const {
	return operator*(-1);
}


float& Mat4x4::operator[](int i) {
	return entries[i];
}

float* Mat4x4::operator&() {
	return entries;
}

Vec4 Mat4x4::operator*(Vec4 in) {
	return {
		in.x * entries[0] + in.y * entries[4] + in.z * entries[8] + in.w * entries[12],
		in.x * entries[1] + in.y * entries[5] + in.z * entries[9] + in.w * entries[13],
		in.x * entries[2] + in.y * entries[6] + in.z * entries[10] + in.w * entries[14],
		in.x * entries[3] + in.y * entries[7] + in.z * entries[11] + in.w * entries[15]
	};
}

Mat4x4 Mat4x4::operator*(Mat4x4& right) {
	Mat4x4& left = *this;
	Mat4x4 out;

	out[0] = left[0] * right[0] + left[4] * right[1] + left[8] * right[2] + left[12] * right[3];
	out[1] = left[1] * right[0] + left[5] * right[1] + left[9] * right[2] + left[13] * right[3];
	out[2] = left[2] * right[0] + left[6] * right[1] + left[10] * right[2] + left[14] * right[3];
	out[3] = left[3] * right[0] + left[7] * right[1] + left[11] * right[2] + left[15] * right[3];

	out[4] = left[0] * right[4] + left[4] * right[5] + left[8] * right[6] + left[12] * right[7];
	out[5] = left[1] * right[4] + left[5] * right[5] + left[9] * right[6] + left[13] * right[7];
	out[6] = left[2] * right[4] + left[6] * right[5] + left[10] * right[6] + left[14] * right[7];
	out[7] = left[3] * right[4] + left[7] * right[5] + left[11] * right[6] + left[15] * right[7];

	out[8] = left[0] * right[8] + left[4] * right[9] + left[8] * right[10] + left[12] * right[11];
	out[9] = left[1] * right[8] + left[5] * right[9] + left[9] * right[10] + left[13] * right[11];
	out[10] = left[2] * right[8] + left[6] * right[9] + left[10] * right[10] + left[14] * right[11];
	out[11] = left[3] * right[8] + left[7] * right[9] + left[11] * right[10] + left[15] * right[11];

	out[12] = left[0] * right[12] + left[4] * right[13] + left[8] * right[14] + left[12] * right[15];
	out[13] = left[1] * right[12] + left[5] * right[13] + left[9] * right[14] + left[13] * right[15];
	out[14] = left[2] * right[12] + left[6] * right[13] + left[10] * right[14] + left[14] * right[15];
	out[15] = left[3] * right[12] + left[7] * right[13] + left[11] * right[14] + left[15] * right[15];

	return out;
}


Mat4x4 view(Vec3 at, Vec3 look, Vec3 world_up) {

	// We are using a right-handed coordinate system; we want to align +w with +z, i.e. "look" should align with -z
	Vec3 w { -look.normalised() };
	Vec3 u { world_up.cross(w).normalised() };
	Vec3 v { w.cross(u).normalised() };

	// Since M[uvw --> xyz] = (u, v, w), N[xyz --> uvw] = M^-1 = M^T (as { u, v, w } is orthonormal ==> (u, v, w) is orthogonal)
	Mat4x4 rotate;

	rotate[0] = u.x;	rotate[4] = u.y;	rotate[8] = u.z;	rotate[12] = 0;
	rotate[1] = v.x;	rotate[5] = v.y;	rotate[9] = v.z;	rotate[13] = 0;
	rotate[2] = w.x;	rotate[6] = w.y;	rotate[10] = w.z;	rotate[14] = 0;
	rotate[3] = 0;		rotate[7] = 0;		rotate[11] = 0;		rotate[15] = 1;

	// Also need to take world origin to camera origin - can combine these steps for efficiency,
	// but they are separated here for readability
	Mat4x4 translate;

	translate[0] = 1;	translate[4] = 0;	translate[8] = 0;	translate[12] = -at.x;
	translate[1] = 0;	translate[5] = 1;	translate[9] = 0;	translate[13] = -at.y;
	translate[2] = 0;	translate[6] = 0;	translate[10] = 1;	translate[14] = -at.z;
	translate[3] = 0;	translate[7] = 0;	translate[11] = 0;	translate[15] = 1;

	return rotate * translate;
}

Mat4x4 perspective(float aspect, float fov, float near, float far) {
	assert(near < far && "Near distance is larger than far distance");
	assert(near >= 0);
	assert(far >= 0);

	float radians { fov * 3.14159f / 180.0f };

	float n { near };
	float f { far };

	float t { tan(radians / 2) * near };
	float r { t * aspect };

	// project onto near plane
	Mat4x4 out;

	out[0] = n / r;	out[4] = 0;		out[8] = 0;					out[12] = 0;
	out[1] = 0;		out[5] = n / t;	out[9] = 0;					out[13] = 0;
	out[2] = 0;		out[6] = 0;		out[10] = (n+f) / (n-f);	out[14] = 2*n*f / (n-f);
	out[3] = 0;		out[7] = 0;		out[11] = -1;				out[15] = 0;
	
	return out;
}

Mat4x4 orthographic(float width, float height, float near_dist, float far_dist) {
	assert(width >= 0 && "Negative width supplied to orthographic projection.");
	assert(height >= 0 && "Negative height supplied to orthographic projection.");
	assert(near_dist >= 0 && "Negative near distance supplied to orthographic projection.");
	assert(far_dist >= 0 && "Negative far distance supplied to orthographic projection.");
	assert(near_dist < far_dist && "Near distance is larger than far distance for orthographic projection.");

	return orthographic(-width / 2, width / 2, -height / 2, height / 2, -far_dist, -near_dist);
}

Mat4x4 orthographic(float l, float r, float b, float t, float f, float n) {
	assert(l < r);
	assert(b < t);
	assert(f < n);

	Mat4x4 out;

	out[0] = 2 / (r - l);	out[4] = 0;				out[8] = 0;				out[12] = -(r + l) / (r - l);
	out[1] = 0;				out[5] = 2 / (t - b);	out[9] = 0;				out[13] = -(t + b) / (t - b);
	out[2] = 0;				out[6] = 0;				out[10] = 2 / (n - f);	out[14] = -(n + f) / (n - f);
	out[3] = 0;				out[7] = 0;				out[11] = 0;			out[15] = 1;

	return out;
}