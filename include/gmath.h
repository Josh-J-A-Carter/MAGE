#ifndef GMATH_H
#define GMATH_H



struct Vec3 {
	float x, y, z;

	Vec3 cross(Vec3 other) const;

	float dot(Vec3 other) const;

	float magnitude() const;

	Vec3 normalised() const;

	Vec3 operator*(float scalar) const;

	Vec3 operator+(Vec3 other) const;

	void operator+=(Vec3 other);

	Vec3 operator-() const;
};

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

Mat4x4 view(Vec3 at, Vec3 look, Vec3 world_up);

Mat4x4 perspective(float aspect, float fov, float near, float far);

Mat4x4 orthographic(float l, float r, float b, float t, float f, float n);

Mat4x4 orthographic(float width, float height, float near_dist, float far_dist);

#endif