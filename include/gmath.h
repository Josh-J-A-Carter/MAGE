#ifndef GMATH_H
#define GMATH_H

#include <string>
#include <iostream>
#include <random>

struct Vec3 {
	float x, y, z;

	Vec3 cross(Vec3 other) const;

	float dot(Vec3 other) const;

	float magnitude() const;

	Vec3 normalised() const;

	float* operator&();

	Vec3 operator*(float scalar) const;

	Vec3 operator+(Vec3 other) const;

	void operator+=(Vec3 other);

	Vec3 operator-() const;

	static Vec3 rand(std::mt19937& generator, float lower, float upper);
};

Vec3 operator*(float scalar, Vec3 vec);

std::ostream& operator<<(std::ostream& stream, Vec3 vec);

struct Vec4 {
	float x, y, z, w;
};

// Note: Column Order
struct Mat4x4 {
	// Note: We assume column-major order, in line with what OpenGL expects
	float entries[16];

	float& operator[](int i);

	float* operator&();

	Vec4 operator*(Vec4 in);

	Mat4x4 operator*(Mat4x4& right);
};


struct Quat {
	float w, x, y, z;

	Quat() : w { 1 }, x { 0 }, y { 0 }, z { 0 } {}

	Quat(float w, float x, float y, float z) : w { w }, x { x }, y { y }, z { z } {}
	
	Quat(float s, Vec3 v) : w { s }, x { v.x }, y { v.y }, z { v.z } {}

	Quat operator*(Quat other) const;

	Quat conjugate() const;

	Mat4x4 matrix() const;

	static Quat rotation(Vec3 axis, float amount);

	static Vec3 rotate_around(Vec3 vec, Vec3 axis, float amount);

	static Quat rotation_between(Vec3 v1, Vec3 v2);
};


Mat4x4 view(Vec3 at, Vec3 look, Vec3 world_up);

Mat4x4 perspective(float aspect, float fov, float near, float far);

Mat4x4 orthographic(float l, float r, float b, float t, float f, float n);

Mat4x4 orthographic(float width, float height, float near_dist, float far_dist);

#endif