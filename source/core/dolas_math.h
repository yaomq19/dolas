#ifndef DOLAS_MATH_H
#define DOLAS_MATH_H
#include "base/dolas_base.h"

namespace Dolas
{
    #define DOLAS_PI (3.14159265358979323846)
    #define DOLAS_FLOAT_MAX (3.40282346638528859812e+38f)
    #define DOLAS_FLOAT_MIN (-3.40282346638528859812e+38f)
    #define DOLAS_FLOAT_EPSILON (1.1920928955078125e-07f)
    #define DOLAS_FLOAT_INFINITY (3.40282346638528859812e+38f)
    #define DOLAS_FLOAT_NEGATIVE_INFINITY (-3.40282346638528859812e+38f)
    #define DOLAS_FLOAT_NAN (0.0f / 0.0f)
    #define DOLAS_FLOAT_POSITIVE_INFINITY (3.40282346638528859812e+38f)
    #define DOLAS_FLOAT_NEGATIVE_INFINITY (-3.40282346638528859812e+38f)
    class Matrix4x4;

    class Vector2
    {
    public:
        Vector2();
        Vector2(Float x, Float y);
        Vector2(const Vector2& other);
        ~Vector2();
        
        Float Length() const;
        Float Dot(const Vector2& other) const;
        Vector2 Normalized() const;
        
        // 运算符重载
        Vector2 operator+(const Vector2& other) const;
        Vector2 operator-(const Vector2& other) const;
        Vector2 operator*(const Vector2& other) const;
        Vector2 operator/(const Vector2& other) const;
        Vector2 operator*(const Float& number) const;
        Vector2 operator/(const Float& number) const;
        
        Vector2& operator+=(const Vector2& other);
        Vector2& operator-=(const Vector2& other);
        Vector2& operator*=(const Vector2& other);
        Vector2& operator/=(const Vector2& other);
        Vector2& operator*=(const Float& number);
        Vector2& operator/=(const Float& number);
        
    public:
        Float x;
        Float y;

    public:
        static const Vector2 ZERO;
        static const Vector2 UNIT_X;
        static const Vector2 UNIT_Y;
        static const Vector2 UNIT_X_NEGATIVE;
        static const Vector2 UNIT_Y_NEGATIVE;
    };

    class Vector3
    {
    public:
        Vector3();
        Vector3(Float x, Float y, Float z);
        Vector3(const Vector3& other);
        ~Vector3();

        Float Length() const;
        Float LengthSquared() const;
        Float Dot(const Vector3& other) const;
        Vector3 Cross(const Vector3& other) const;
        void Normalize();
        Vector3 Normalized() const;

        Float& operator[](UInt index);
        const Float& operator[](UInt index) const;

        Vector3 operator+(const Vector3& other) const;
        Vector3 operator-(const Vector3& other) const;
        Vector3 operator*(const Vector3& other) const;
        Vector3 operator/(const Vector3& other) const;
        Vector3 operator*(const Float& number) const;
        Vector3 operator/(const Float& number) const;

        Vector3& operator+=(const Vector3& other);
        Vector3& operator-=(const Vector3& other);
        Vector3& operator*=(const Vector3& other);
        Vector3& operator/=(const Vector3& other);
        Vector3& operator*=(const Float& number);
        Vector3& operator/=(const Float& number);
		Vector3 operator-() const;

    public:
        Float x;
        Float y;
        Float z;

    public:
        static const Vector3 ZERO;
        static const Vector3 UNIT_X;
        static const Vector3 UNIT_Y;
        static const Vector3 UNIT_Z;
        static const Vector3 UNIT_X_NEGATIVE;
        static const Vector3 UNIT_Y_NEGATIVE;
        static const Vector3 UNIT_Z_NEGATIVE;
    };

    class Vector4
    {
    public:
        Vector4();
        Vector4(Float x, Float y, Float z, Float w);
        Vector4(const Vector3& other, Float w);
        Vector4(const Vector4& other);
        ~Vector4();

        Float Length() const;
        Float LengthSquared() const;
        Float Dot(const Vector4& other) const;
        Vector4 Cross(const Vector4& other) const;
        void Normalize();

        Float& operator[](UInt index);
        const Float& operator[](UInt index) const;
        Vector4 operator+(const Vector4& other) const;
        Vector4 operator-(const Vector4& other) const;
        Vector4 operator*(const Vector4& other) const;
        Vector4 operator/(const Vector4& other) const;
        Vector4 operator*(const Float& number) const;
        Vector4 operator/(const Float& number) const;

        Vector4& operator+=(const Vector4& other);
        Vector4& operator-=(const Vector4& other);
        Vector4& operator*=(const Vector4& other);
        Vector4& operator/=(const Vector4& other);
        Vector4& operator*=(const Float& number);
        Vector4& operator/=(const Float& number);

    public:
        Float x;
        Float y;
        Float z;
        Float w;

    public:
        static const Vector4 ZERO;
    };

    class Matrix3x3
    {
    public:
        Matrix3x3();
        Matrix3x3(Float m00, Float m01, Float m02, Float m10, Float m11, Float m12, Float m20, Float m21, Float m22);
        Matrix3x3(const Matrix3x3& other);
        ~Matrix3x3();

        Matrix3x3 operator+(const Matrix3x3& other) const;
        Matrix3x3 operator*(const Matrix3x3& other) const;
        Matrix3x3 operator*(const Float& number) const;

        Matrix3x3& operator*=(const Matrix3x3& other);
        Matrix3x3& operator*=(const Float& number);

        Matrix3x3& operator+=(const Matrix3x3& other);
        Matrix3x3& operator-=(const Matrix3x3& other);

        Matrix3x3& operator+=(const Float& number);
        Matrix3x3& operator-=(const Float& number);

        Vector3 operator*(const Vector3& other) const;

        Vector3 GetRow(UInt index) const;
        Vector3 GetColumn(UInt index) const;

        void SetRow(UInt index, const Vector3& value);
        void SetColumn(UInt index, const Vector3& value);

        void SetIdentity();
        void SetZero();
        Matrix3x3 GetTranspose() const;
        Matrix3x3 GetInverse() const;

        Matrix4x4 ExpandToMatrix4x4() const;

    public:
        Float data[3][3];

    public:
        static const Matrix3x3 IDENTITY;
        static const Matrix3x3 ZERO;
    };

    class Matrix4x4
    {
    public:
        Matrix4x4();
        Matrix4x4(Float m00, Float m01, Float m02, Float m03, Float m10, Float m11, Float m12, Float m13, Float m20, Float m21, Float m22, Float m23, Float m30, Float m31, Float m32, Float m33);
        Matrix4x4(const Matrix4x4& other);
        ~Matrix4x4();

        Matrix4x4 operator*(const Matrix4x4& other) const;
        Matrix4x4 operator*(const Float& number) const;

        Matrix4x4& operator*=(const Matrix4x4& other);
        Matrix4x4& operator*=(const Float& number);

        Matrix4x4& operator+=(const Matrix4x4& other);
        Matrix4x4& operator-=(const Matrix4x4& other);

        Matrix4x4& operator+=(const Float& number);
        Matrix4x4& operator-=(const Float& number);

        Vector4 operator*(const Vector4& other) const;

        Vector4 GetRow(UInt index) const;
        Vector4 GetColumn(UInt index) const;

        void SetRow(UInt index, const Vector4& value);
        void SetColumn(UInt index, const Vector4& value);

        void SetZero();
        void SetIdentity();

        Matrix4x4 GetTranspose() const;
        Matrix4x4 GetInverse() const;

        // l, r, t, b, f, n is Coordinate, so f = -far_plane, n = -near_plane
        // so Even though near_plane < far_plane but f < n
        static Matrix4x4 Orthographic(Float l, Float r, Float t, Float b, Float f, Float n);
        // fov in radian
        // f, n is Coordinate, so f = -far_plane, n = -near_plane
        // so Even though near_plane < far_plane but f < n
        static Matrix4x4 Perspective(Float fov, Float aspectRatio, Float f, Float n);
    public:
        Float data[4][4];
    public:
        static const Matrix4x4 IDENTITY;
        static const Matrix4x4 ZERO;
    };

    class MathUtil
    {
    public:
        static Matrix3x3 Rotate(const Vector3& axis, Float angle);
        static Float DegreesToRadians(Float degrees);
        static Float RadiansToDegrees(Float radians);
        static const Float PI;
    };
    
    // scalar * matrix helpers
    inline Matrix3x3 operator*(Float scalar, const Matrix3x3& m)
    {
        return m * scalar;
    }

    inline Matrix4x4 operator*(Float scalar, const Matrix4x4& m)
    {
        return m * scalar;
    }

	class Quaternion
	{
    public:
        Quaternion();
        Quaternion(Float w, Float x, Float y, Float z);
		Float w;
		Float x;
		Float y;
		Float z;
	    
        // static const value
        static const Quaternion IDENTITY;
	};

	struct Pose
	{
		Pose() : m_postion(0.0, 0.0, 0.0), m_rotation(1.0f, 0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f)
        {

        }

		Pose(const Vector3& position, const Quaternion& rotation, const Vector3& scale) : m_postion(position), m_rotation(rotation), m_scale(scale)
        {

        }
		Vector3 m_postion;
		Quaternion m_rotation; // 
		Vector3 m_scale;
	};
}
#endif // DOLAS_MATH_H