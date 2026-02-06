#include <cmath>
#include "dolas_math.h"

namespace Dolas
{
    /* Quaternion */
    Quaternion::Quaternion(): w(1.0f), x(0.0f), y(0.0f), z(0.0f)
    {
    }

    Quaternion::Quaternion(Float w, Float x, Float y, Float z): w(w), x(x), y(y), z(z)
    {
    }

    Quaternion::Quaternion(const Vector3& axis, Float angle)
    {
        // axis 为旋转轴，angle 为角度（单位：度）
        // 约定：从原点沿着 axis 方向看过去，顺时针为正角度。
        // 而四元数的标准右手系约定是“逆时针为正角度”，
        // 因此这里需要把角度取反，再生成标准右手系四元数。

        // 1. 归一化旋转轴
        Float len = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
        if (len <= 0.0f)
        {
            // 退化情况：轴长度为 0，则使用单位四元数
            w = 1.0f;
            x = y = z = 0.0f;
            return;
        }

        Float nx = axis.x / len;
        Float ny = axis.y / len;
        Float nz = axis.z / len;

        // 2. 角度（度）转成弧度，并取反实现“从轴方向看去顺时针为正”
        Float radians    = MathUtil::DegreesToRadians(angle);
        Float half_angle = -0.5f * radians; // 负号实现顺时针为正

        Float s = std::sin(half_angle);
        Float c = std::cos(half_angle);

        // 3. 标准四元数形式：q = [cos(θ/2), sin(θ/2) * n]
        w = c;
        x = nx * s;
        y = ny * s;
        z = nz * s;
    }

    const Quaternion Quaternion::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f);

    // Vector2 implementation
    Vector2::Vector2() : x(0.0f), y(0.0f) {}
    
    Vector2::Vector2(Float x, Float y) : x(x), y(y) {}
    
    Vector2::Vector2(const Vector2& other) : x(other.x), y(other.y) {}
    
    Vector2::~Vector2() {}
    
    Float Vector2::Length() const
    {
        return std::sqrt(x * x + y * y);
    }
    
    Float Vector2::Dot(const Vector2& other) const
    {
        return x * other.x + y * other.y;
    }
    
    Vector2 Vector2::Normalized() const
    {
        Float length = Length();
        if (length > 0.0f)
        {
            return {x / length, y / length};
        }
        return {0.0f, 0.0f};
    }
    
    Vector2 Vector2::operator+(const Vector2& other) const
    {
        return {x + other.x, y + other.y};
    }
    
    Vector2 Vector2::operator-(const Vector2& other) const
    {
        return {x - other.x, y - other.y};
    }
    
    Vector2 Vector2::operator*(const Vector2& other) const
    {
        return {x * other.x, y * other.y};
    }
    
    Vector2 Vector2::operator/(const Vector2& other) const
    {
        return {x / other.x, y / other.y};
    }
    
    Vector2 Vector2::operator*(const Float& number) const
    {
        return {x * number, y * number};
    }
    
    Vector2 Vector2::operator/(const Float& number) const
    {
        return {x / number, y / number};
    }
    
    Vector2& Vector2::operator+=(const Vector2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Vector2& Vector2::operator-=(const Vector2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    
    Vector2& Vector2::operator*=(const Vector2& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }
    
    Vector2& Vector2::operator/=(const Vector2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }
    
    Vector2& Vector2::operator*=(const Float& number)
    {
        x *= number;
        y *= number;
        return *this;
    }
    
    Vector2& Vector2::operator/=(const Float& number)
    {
        x /= number;
        y /= number;
        return *this;
    }
    
    const Vector2 Vector2::ZERO(0.0f, 0.0f);
    const Vector2 Vector2::UNIT_X(1.0f, 0.0f);
    const Vector2 Vector2::UNIT_Y(0.0f, 1.0f);
    const Vector2 Vector2::UNIT_X_NEGATIVE(-1.0f, 0.0f);
    const Vector2 Vector2::UNIT_Y_NEGATIVE(0.0f, -1.0f);
    
    // Vector3 implementation
    Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f)
    {

    }

    Vector3::Vector3(Float x, Float y, Float z) : x(x), y(y), z(z)
    {

    }
    
    Vector3::Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z)
    {
    }

    Vector3::~Vector3()
    {
    }

    Float Vector3::Length() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    Float Vector3::LengthSquared() const
    {
        return x * x + y * y + z * z;
    }

    Float Vector3::Dot(const Vector3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 Vector3::Cross(const Vector3& other) const
    {
        return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    void Vector3::Normalize()
    {
        this->x /= Length();
        this->y /= Length();
        this->z /= Length();
    }

    Vector3 Vector3::Normalized() const
    {
        return Vector3(x / Length(), y / Length(), z / Length());
    }

    Float& Vector3::operator[](UInt index)
    {
        return index == 0 ? x : index == 1 ? y : z;
    }

    const Float& Vector3::operator[](UInt index) const
    {
        return index == 0 ? x : index == 1 ? y : z;
    }

    Vector3 Vector3::operator+(const Vector3& other) const
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 Vector3::operator-(const Vector3& other) const
    {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 Vector3::operator*(const Vector3& other) const
    {
        return Vector3(x * other.x, y * other.y, z * other.z);
    }

    Vector3 Vector3::operator/(const Vector3& other) const
    {
        return Vector3(x / other.x, y / other.y, z / other.z);
    }
    
    Vector3 Vector3::operator*(const Float& number) const
    {
        return Vector3(x * number, y * number, z * number);
    }

    Vector3 Vector3::operator/(const Float& number) const
    {
        return Vector3(x / number, y / number, z / number);
    }
    
    Vector3& Vector3::operator+=(const Vector3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& Vector3::operator-=(const Vector3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& Vector3::operator*=(const Vector3& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }
    
    Vector3& Vector3::operator/=(const Vector3& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }
    
    Vector3& Vector3::operator*=(const Float& number)
    {
        x *= number;
        y *= number;
        z *= number;
        return *this;
    }
    
    Vector3& Vector3::operator/=(const Float& number)
    {
        x /= number;
        y /= number;
        z /= number;
        return *this;
    }
    
    Vector3 Vector3::operator-() const
    {
		return Vector3(-x, -y, -z);
    }

    const Vector3 Vector3::ZERO(0.0f, 0.0f, 0.0f);
    const Vector3 Vector3::UNIT_X(1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::UNIT_Y(0.0f, 1.0f, 0.0f);
    const Vector3 Vector3::UNIT_Z(0.0f, 0.0f, 1.0f);
    const Vector3 Vector3::UNIT_X_NEGATIVE(-1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::UNIT_Y_NEGATIVE(0.0f, -1.0f, 0.0f);
    const Vector3 Vector3::UNIT_Z_NEGATIVE(0.0f, 0.0f, -1.0f);

    /* Vector4 */
    Vector4::Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
    {
    }

    Vector4::Vector4(Float x, Float y, Float z, Float w) : x(x), y(y), z(z), w(w)
    {
    }

    Vector4::Vector4(const Vector3& other, Float w): x(other.x), y(other.y), z(other.z), w(w)
    {

    }


    Vector4::Vector4(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w)
    {
    }

    Vector4::~Vector4()
    {
    }

    Float Vector4::Length() const
    {
        return sqrt(x * x + y * y + z * z + w * w);
    }

    Float Vector4::LengthSquared() const
    {
        return x * x + y * y + z * z + w * w;
    }

    Float Vector4::Dot(const Vector4& other) const
    {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    Vector4 Vector4::Cross(const Vector4& other) const
    {
        return Vector4(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x, w * other.w);
    }

    void Vector4::Normalize()
    {
        this->x /= Length();
        this->y /= Length();
        this->z /= Length();
        this->w /= Length();
    }

    Float& Vector4::operator[](UInt index)
    {
        return index == 0 ? x : index == 1 ? y : index == 2 ? z : w;
    }

    const Float& Vector4::operator[](UInt index) const
    {
        return index == 0 ? x : index == 1 ? y : index == 2 ? z : w;
    }

    Vector4 Vector4::operator+(const Vector4& other) const
    {
        return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    Vector4 Vector4::operator-(const Vector4& other) const
    {
        return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    Vector4 Vector4::operator*(const Vector4& other) const
    {
        return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    Vector4 Vector4::operator/(const Vector4& other) const
    {
        return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    Vector4 Vector4::operator*(const Float& number) const
    {
        return Vector4(x * number, y * number, z * number, w * number);
    }

    Vector4 Vector4::operator/(const Float& number) const
    {
        return Vector4(x / number, y / number, z / number, w / number);
    }

    Vector4& Vector4::operator+=(const Vector4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4& Vector4::operator-=(const Vector4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Vector4& Vector4::operator*=(const Vector4& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }
    
    Vector4& Vector4::operator/=(const Vector4& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }
    
    
    Vector4& Vector4::operator*=(const Float& number)
    {
        x *= number;
        y *= number;
        z *= number;
        w *= number;
        return *this;
    }
    
    
    Vector4& Vector4::operator/=(const Float& number)
    {
        x /= number;
        y /= number;
        z /= number;
        w /= number;
        return *this;
    }
    
    const Vector4 Vector4::ZERO(0.0f, 0.0f, 0.0f, 0.0f);
    /* Matrix3x3 */
    Matrix3x3::Matrix3x3()
    {
        SetZero();
    }

    Matrix3x3::Matrix3x3(Float m00, Float m01, Float m02, Float m10, Float m11, Float m12, Float m20, Float m21, Float m22)
    {
        data[0][0] = m00;
        data[0][1] = m01;
        data[0][2] = m02;
        data[1][0] = m10;
        data[1][1] = m11;
        data[1][2] = m12;
        data[2][0] = m20;
        data[2][1] = m21;
        data[2][2] = m22;
    }

    Matrix3x3::Matrix3x3(const Matrix3x3& other)
    {
        data[0][0] = other.data[0][0];
        data[0][1] = other.data[0][1];
        data[0][2] = other.data[0][2];
        data[1][0] = other.data[1][0];
        data[1][1] = other.data[1][1];
        data[1][2] = other.data[1][2];
        data[2][0] = other.data[2][0];
        data[2][1] = other.data[2][1];
        data[2][2] = other.data[2][2];
    }

    Matrix3x3::~Matrix3x3()
    {
    }

    Matrix3x3 Matrix3x3::operator+(const Matrix3x3& other) const
    {
        return Matrix3x3(
            data[0][0] + other.data[0][0], data[0][1] + other.data[0][1], data[0][2] + other.data[0][2],
            data[1][0] + other.data[1][0], data[1][1] + other.data[1][1], data[1][2] + other.data[1][2],
            data[2][0] + other.data[2][0], data[2][1] + other.data[2][1], data[2][2] + other.data[2][2]);
    }

    Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const
    {
        return Matrix3x3(
            data[0][0] * other.data[0][0] + data[0][1] * other.data[1][0] + data[0][2] * other.data[2][0],
            data[0][0] * other.data[0][1] + data[0][1] * other.data[1][1] + data[0][2] * other.data[2][1],
            data[0][0] * other.data[0][2] + data[0][1] * other.data[1][2] + data[0][2] * other.data[2][2],
            data[1][0] * other.data[0][0] + data[1][1] * other.data[1][0] + data[1][2] * other.data[2][0],
            data[1][0] * other.data[0][1] + data[1][1] * other.data[1][1] + data[1][2] * other.data[2][1],
            data[1][0] * other.data[0][2] + data[1][1] * other.data[1][2] + data[1][2] * other.data[2][2],
            data[2][0] * other.data[0][0] + data[2][1] * other.data[1][0] + data[2][2] * other.data[2][0],
            data[2][0] * other.data[0][1] + data[2][1] * other.data[1][1] + data[2][2] * other.data[2][1],
            data[2][0] * other.data[0][2] + data[2][1] * other.data[1][2] + data[2][2] * other.data[2][2]);
    }

    Matrix3x3 Matrix3x3::operator*(const Float& number) const
    {
        return Matrix3x3(
            data[0][0] * number,
            data[0][1] * number,
            data[0][2] * number,
            data[1][0] * number,
            data[1][1] * number,
            data[1][2] * number,
            data[2][0] * number,
            data[2][1] * number,
            data[2][2] * number);
    }

    Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& other)
    {
        *this = *this * other;
        return *this;
    }

    Matrix3x3& Matrix3x3::operator*=(const Float& number)
    {
        *this = *this * number;
        return *this;
    }

    Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& other)
    {
        data[0][0] += other.data[0][0];
        data[0][1] += other.data[0][1];
        data[0][2] += other.data[0][2];
        data[1][0] += other.data[1][0];
        data[1][1] += other.data[1][1];
        data[1][2] += other.data[1][2];
        data[2][0] += other.data[2][0];
        data[2][1] += other.data[2][1];
        data[2][2] += other.data[2][2];
        return *this;
    }

    Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& other)
    {
        data[0][0] -= other.data[0][0];
        data[0][1] -= other.data[0][1];
        data[0][2] -= other.data[0][2];
        data[1][0] -= other.data[1][0];
        data[1][1] -= other.data[1][1];
        data[1][2] -= other.data[1][2];
        data[2][0] -= other.data[2][0];
        data[2][1] -= other.data[2][1];
        data[2][2] -= other.data[2][2];
        return *this;
    }

    Matrix3x3& Matrix3x3::operator+=(const Float& number)
    {
        data[0][0] += number;
        data[0][1] += number;
        data[0][2] += number;
        data[1][0] += number;
        data[1][1] += number;
        data[1][2] += number;
        data[2][0] += number;
        data[2][1] += number;
        data[2][2] += number;
        return *this;
    }

    Matrix3x3& Matrix3x3::operator-=(const Float& number)
    {
        data[0][0] -= number;
        data[0][1] -= number;
        data[0][2] -= number;
        data[1][0] -= number;
        data[1][1] -= number;
        data[1][2] -= number;
        data[2][0] -= number;
        data[2][1] -= number;
        data[2][2] -= number;
        return *this;
    }

    Vector3 Matrix3x3::operator*(const Vector3& other) const
    {
        return Vector3(
            data[0][0] * other.x + data[0][1] * other.y + data[0][2] * other.z,
            data[1][0] * other.x + data[1][1] * other.y + data[1][2] * other.z,
            data[2][0] * other.x + data[2][1] * other.y + data[2][2] * other.z);
    }

    Vector3 Matrix3x3::GetRow(UInt index) const
    {
        return Vector3(data[index][0], data[index][1], data[index][2]);
    }

    Vector3 Matrix3x3::GetColumn(UInt index) const
    {
        return Vector3(data[0][index], data[1][index], data[2][index]);
    }

    void Matrix3x3::SetRow(UInt index, const Vector3& value)
    {
        data[index][0] = value.x;
        data[index][1] = value.y;
        data[index][2] = value.z;
    }

    void Matrix3x3::SetColumn(UInt index, const Vector3& value)
    {
        data[0][index] = value.x;
        data[1][index] = value.y;
        data[2][index] = value.z;
    }
    
    void Matrix3x3::SetIdentity()
    {
        data[0][0] = 1.0f;
        data[0][1] = 0.0f;
        data[0][2] = 0.0f;
        data[1][0] = 0.0f;
        data[1][1] = 1.0f;
        data[1][2] = 0.0f;
        data[2][0] = 0.0f;
        data[2][1] = 0.0f;
        data[2][2] = 1.0f;
    }
    
    void Matrix3x3::SetZero()
    {
        data[0][0] = 0.0f;
        data[0][1] = 0.0f;
        data[0][2] = 0.0f;
        data[1][0] = 0.0f;
        data[1][1] = 0.0f;
        data[1][2] = 0.0f;
        data[2][0] = 0.0f;
        data[2][1] = 0.0f;
        data[2][2] = 0.0f;
    }

    Matrix3x3 Matrix3x3::GetTranspose() const
    {
        return Matrix3x3(
            data[0][0], data[1][0], data[2][0],
            data[0][1], data[1][1], data[2][1],
            data[0][2], data[1][2], data[2][2]);
    }
    
    Matrix3x3 Matrix3x3::GetInverse() const
    {
        return Matrix3x3(
            data[0][0], data[1][0], data[2][0],
            data[0][1], data[1][1], data[2][1],
            data[0][2], data[1][2], data[2][2]);
    }
    
    Matrix4x4 Matrix3x3::ExpandToMatrix4x4() const
    {
        return Matrix4x4(
            data[0][0], data[0][1], data[0][2], 0.0f,
            data[1][0], data[1][1], data[1][2], 0.0f,
            data[2][0], data[2][1], data[2][2], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    const Matrix3x3 Matrix3x3::IDENTITY(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f);
    const Matrix3x3 Matrix3x3::ZERO(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    /* Matrix4x4 */
    Matrix4x4::Matrix4x4()
    {
        SetZero();
    }
    
    Matrix4x4::Matrix4x4(Float m00, Float m01, Float m02, Float m03, Float m10, Float m11, Float m12, Float m13, Float m20, Float m21, Float m22, Float m23, Float m30, Float m31, Float m32, Float m33)
    {
        data[0][0] = m00;
        data[0][1] = m01;
        data[0][2] = m02;
        data[0][3] = m03;
        data[1][0] = m10;
        data[1][1] = m11;
        data[1][2] = m12;
        data[1][3] = m13;
        data[2][0] = m20;
        data[2][1] = m21;
        data[2][2] = m22;
        data[2][3] = m23;
        data[3][0] = m30;
        data[3][1] = m31;
        data[3][2] = m32;
        data[3][3] = m33;
    }

    Matrix4x4::Matrix4x4(const Matrix4x4& other)
    {
        data[0][0] = other.data[0][0];
        data[0][1] = other.data[0][1];
        data[0][2] = other.data[0][2];
        data[0][3] = other.data[0][3];
        data[1][0] = other.data[1][0];
        data[1][1] = other.data[1][1];
        data[1][2] = other.data[1][2];
        data[1][3] = other.data[1][3];
        data[2][0] = other.data[2][0];
        data[2][1] = other.data[2][1];
        data[2][2] = other.data[2][2];
        data[2][3] = other.data[2][3];
        data[3][0] = other.data[3][0];
        data[3][1] = other.data[3][1];
        data[3][2] = other.data[3][2];
        data[3][3] = other.data[3][3];
    }

    Matrix4x4::~Matrix4x4()
    {
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
    {
        return Matrix4x4(
            data[0][0] * other.data[0][0] + data[0][1] * other.data[1][0] + data[0][2] * other.data[2][0] + data[0][3] * other.data[3][0],
            data[0][0] * other.data[0][1] + data[0][1] * other.data[1][1] + data[0][2] * other.data[2][1] + data[0][3] * other.data[3][1],
            data[0][0] * other.data[0][2] + data[0][1] * other.data[1][2] + data[0][2] * other.data[2][2] + data[0][3] * other.data[3][2],
            data[0][0] * other.data[0][3] + data[0][1] * other.data[1][3] + data[0][2] * other.data[2][3] + data[0][3] * other.data[3][3],
            data[1][0] * other.data[0][0] + data[1][1] * other.data[1][0] + data[1][2] * other.data[2][0] + data[1][3] * other.data[3][0],
            data[1][0] * other.data[0][1] + data[1][1] * other.data[1][1] + data[1][2] * other.data[2][1] + data[1][3] * other.data[3][1],
            data[1][0] * other.data[0][2] + data[1][1] * other.data[1][2] + data[1][2] * other.data[2][2] + data[1][3] * other.data[3][2],
            data[1][0] * other.data[0][3] + data[1][1] * other.data[1][3] + data[1][2] * other.data[2][3] + data[1][3] * other.data[3][3],
            data[2][0] * other.data[0][0] + data[2][1] * other.data[1][0] + data[2][2] * other.data[2][0] + data[2][3] * other.data[3][0],
            data[2][0] * other.data[0][1] + data[2][1] * other.data[1][1] + data[2][2] * other.data[2][1] + data[2][3] * other.data[3][1],
            data[2][0] * other.data[0][2] + data[2][1] * other.data[1][2] + data[2][2] * other.data[2][2] + data[2][3] * other.data[3][2],
            data[2][0] * other.data[0][3] + data[2][1] * other.data[1][3] + data[2][2] * other.data[2][3] + data[2][3] * other.data[3][3],
            data[3][0] * other.data[0][0] + data[3][1] * other.data[1][0] + data[3][2] * other.data[2][0] + data[3][3] * other.data[3][0],
            data[3][0] * other.data[0][1] + data[3][1] * other.data[1][1] + data[3][2] * other.data[2][1] + data[3][3] * other.data[3][1],
            data[3][0] * other.data[0][2] + data[3][1] * other.data[1][2] + data[3][2] * other.data[2][2] + data[3][3] * other.data[3][2],
            data[3][0] * other.data[0][3] + data[3][1] * other.data[1][3] + data[3][2] * other.data[2][3] + data[3][3] * other.data[3][3]);
    }

    Matrix4x4 Matrix4x4::operator*(const Float& number) const
    {
        return Matrix4x4(
            data[0][0] * number,
            data[0][1] * number,
            data[0][2] * number,
            data[0][3] * number,
            data[1][0] * number,
            data[1][1] * number,
            data[1][2] * number,
            data[1][3] * number,
            data[2][0] * number,
            data[2][1] * number,
            data[2][2] * number,
            data[2][3] * number,
            data[3][0] * number,
            data[3][1] * number,
            data[3][2] * number,
            data[3][3] * number);
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other)
    {
        *this = *this * other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator*=(const Float& number)
    {
        *this = *this * number;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other)
    {
        data[0][0] += other.data[0][0];
        data[0][1] += other.data[0][1];
        data[0][2] += other.data[0][2];
        data[0][3] += other.data[0][3];
        data[1][0] += other.data[1][0];
        data[1][1] += other.data[1][1];
        data[1][2] += other.data[1][2];
        data[1][3] += other.data[1][3];
        data[2][0] += other.data[2][0];
        data[2][1] += other.data[2][1];
        data[2][2] += other.data[2][2];
        data[2][3] += other.data[2][3];
        data[3][0] += other.data[3][0];
        data[3][1] += other.data[3][1];
        data[3][2] += other.data[3][2];
        data[3][3] += other.data[3][3];
        return *this;
    }

    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other)
    {
        data[0][0] -= other.data[0][0];
        data[0][1] -= other.data[0][1];
        data[0][2] -= other.data[0][2];
        data[0][3] -= other.data[0][3];
        data[1][0] -= other.data[1][0];
        data[1][1] -= other.data[1][1];
        data[1][2] -= other.data[1][2];
        data[1][3] -= other.data[1][3];
        data[2][0] -= other.data[2][0];
        data[2][1] -= other.data[2][1];
        data[2][2] -= other.data[2][2];
        data[2][3] -= other.data[2][3];
        data[3][0] -= other.data[3][0];
        data[3][1] -= other.data[3][1];
        data[3][2] -= other.data[3][2];
        data[3][3] -= other.data[3][3];
        return *this;
    }

    Matrix4x4& Matrix4x4::operator+=(const Float& number)
    {
        data[0][0] += number;
        data[0][1] += number;
        data[0][2] += number;
        data[0][3] += number;
        data[1][0] += number;
        data[1][1] += number;
        data[1][2] += number;
        data[1][3] += number;
        data[2][0] += number;
        data[2][1] += number;
        data[2][2] += number;
        data[2][3] += number;
        data[3][0] += number;
        data[3][1] += number;
        data[3][2] += number;
        data[3][3] += number;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator-=(const Float& number)
    {
        data[0][0] -= number;
        data[0][1] -= number;
        data[0][2] -= number;
        data[0][3] -= number;
        data[1][0] -= number;
        data[1][1] -= number;
        data[1][2] -= number;
        data[1][3] -= number;
        data[2][0] -= number;
        data[2][1] -= number;
        data[2][2] -= number;
        data[2][3] -= number;
        data[3][0] -= number;
        data[3][1] -= number;
        data[3][2] -= number;
        data[3][3] -= number;
        return *this;
    }

    Vector4 Matrix4x4::operator*(const Vector4& other) const
    {
        return Vector4(
            data[0][0] * other.x + data[0][1] * other.y + data[0][2] * other.z + data[0][3] * other.w,
            data[1][0] * other.x + data[1][1] * other.y + data[1][2] * other.z + data[1][3] * other.w,
            data[2][0] * other.x + data[2][1] * other.y + data[2][2] * other.z + data[2][3] * other.w,
            data[3][0] * other.x + data[3][1] * other.y + data[3][2] * other.z + data[3][3] * other.w);
    }
    
    Vector4 Matrix4x4::GetRow(UInt index) const
    {
        return Vector4(data[index][0], data[index][1], data[index][2], data[index][3]);
    }

    Vector4 Matrix4x4::GetColumn(UInt index) const
    {
        return Vector4(data[0][index], data[1][index], data[2][index], data[3][index]);
    }

    void Matrix4x4::SetRow(UInt index, const Vector4& value)
    {
        data[index][0] = value.x;
        data[index][1] = value.y;
        data[index][2] = value.z;
        data[index][3] = value.w;
    }

    void Matrix4x4::SetColumn(UInt index, const Vector4& value)
    {
        data[0][index] = value.x;
        data[1][index] = value.y;
        data[2][index] = value.z;
        data[3][index] = value.w;
    }

    void Matrix4x4::SetIdentity()
    {
        data[0][0] = 1.0f;
        data[0][1] = 0.0f;
        data[0][2] = 0.0f;
        data[0][3] = 0.0f;
        data[1][0] = 0.0f;
        data[1][1] = 1.0f;
        data[1][2] = 0.0f;
        data[1][3] = 0.0f;
        data[2][0] = 0.0f;
        data[2][1] = 0.0f;
        data[2][2] = 1.0f;
        data[2][3] = 0.0f;
        data[3][0] = 0.0f;
        data[3][1] = 0.0f;
        data[3][2] = 0.0f;
        data[3][3] = 1.0f;
    }

    void Matrix4x4::SetZero()
    {
        data[0][0] = 0.0f;
        data[0][1] = 0.0f;
        data[0][2] = 0.0f;
        data[0][3] = 0.0f;
        data[1][0] = 0.0f;
        data[1][1] = 0.0f;
        data[1][2] = 0.0f;
        data[1][3] = 0.0f;
        data[2][0] = 0.0f;
        data[2][1] = 0.0f;
        data[2][2] = 0.0f;
        data[2][3] = 0.0f;
        data[3][0] = 0.0f;
        data[3][1] = 0.0f;
        data[3][2] = 0.0f;
        data[3][3] = 0.0f;
    }

    Matrix4x4 Matrix4x4::GetTranspose() const
    {
        return Matrix4x4(
            data[0][0], data[1][0], data[2][0], data[3][0],
            data[0][1], data[1][1], data[2][1], data[3][1],
            data[0][2], data[1][2], data[2][2], data[3][2],
            data[0][3], data[1][3], data[2][3], data[3][3]);
    }

    Matrix4x4 Matrix4x4::GetInverse() const
    {
        return Matrix4x4(
            data[0][0], data[1][0], data[2][0], data[3][0],
            data[0][1], data[1][1], data[2][1], data[3][1],
            data[0][2], data[1][2], data[2][2], data[3][2],
            data[0][3], data[1][3], data[2][3], data[3][3]);
    }

    Matrix4x4 Matrix4x4::Orthographic(Float l, Float r, Float t, Float b, Float f, Float n)
    {
        Matrix4x4 translation_matrix(
            1.0f, 0.0f, 0.0f, -(r + l) / 2.0f,
            0.0f, 1.0f, 0.0f, -(t + b) / 2.0f,
            0.0f, 0.0f, 1.0f, -(n + f) / 2.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        Matrix4x4 scale_matrix(
            2.0f / (r - l), 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / (t - b), 0.0f, 0.0f,
            0.0f, 0.0f, 2.0f / (n - f), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

		Matrix4x4 standard_orthographic_matrix = scale_matrix * translation_matrix;

        Matrix4x4 scale_again_matrix(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

        Matrix4x4 translation_again_matrix(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -0.5f,
			0.0f, 0.0f, 0.0f, 1.0f
        );

        Matrix4x4 filp_z_matrix(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );

		Matrix4x4 standard_to_directx_matrix = filp_z_matrix *  translation_again_matrix * scale_again_matrix;

        return standard_to_directx_matrix * standard_orthographic_matrix;
    }
    
    // fov in radian
    Matrix4x4 Matrix4x4::Perspective(Float fov, Float aspectRatio, Float f, Float n)
    {
        Matrix4x4 persp_to_ortho_matrix(
            n, 0.0f, 0.0f, 0.0f,
            0.0f, n, 0.0f, 0.0f,
            0.0f, 0.0f, n + f, -n * f,
            0.0f, 0.0f, 1.0f, 0.0f
        );

        Float near_plane_abs = -n;
        Float half_width = near_plane_abs * std::tan(fov / 2.0f);
        Float half_height = half_width / aspectRatio;
        Float l = -half_width;
        Float r = half_width;
        Float t = half_height;
        Float b = -half_height;
        Matrix4x4 ortho_matrix = Matrix4x4::Orthographic(l, r, t, b, f, n);

        Matrix4x4 adjust_w_matrix(
            -1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, -1.0f
        );
        return adjust_w_matrix * ortho_matrix * persp_to_ortho_matrix;
    }
    
    const Matrix4x4 Matrix4x4::IDENTITY(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    const Matrix4x4 Matrix4x4::ZERO(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    /* MathUtil */
    // angle is radian
    Matrix3x3 MathUtil::Rotate(const Vector3& axis, Float angle)
    {
        // Rodrigues' rotation formula: R = cosθ I + (1-cosθ) kk^T + sinθ [k]_x
        Vector3 n = axis.Normalized();

        Matrix3x3 M1 = std::cos(angle) * Matrix3x3::IDENTITY;

        Matrix3x3 nnT(
            n.x * n.x, n.x * n.y, n.x * n.z,
            n.y * n.x, n.y * n.y, n.y * n.z,
            n.z * n.x, n.z * n.y, n.z * n.z);
        Matrix3x3 M2 = (1 - std::cos(angle)) * nnT;

        Matrix3x3 K(
            0.0f, -n.z,  n.y,
            n.z,  0.0f, -n.x,
           -n.y,  n.x,  0.0f);
        Matrix3x3 M3 = std::sin(angle) * K;

        return M1 + M2 + M3;
    }

    Float MathUtil::DegreesToRadians(Float degrees)
    {
        return degrees * (DOLAS_PI / 180.0f);
    }

    Float MathUtil::RadiansToDegrees(Float radians)
    {
        return radians * (180.0f / DOLAS_PI);
    }

	const Float MathUtil::PI = 3.14159265358979323846f;
}