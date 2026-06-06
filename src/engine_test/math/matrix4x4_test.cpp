#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ Constructor Tests ============

TEST_CASE("Matrix4x4 default constructor", "[Matrix4x4][constructor]")
{
    Matrix4x4 m;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(m.data[i][j] == 0.0f);
}

TEST_CASE("Matrix4x4 16-parameter constructor", "[Matrix4x4][constructor]")
{
    Matrix4x4 m(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    REQUIRE(m.data[0][0] == 1.0f);
    REQUIRE(m.data[0][1] == 2.0f);
    REQUIRE(m.data[0][2] == 3.0f);
    REQUIRE(m.data[0][3] == 4.0f);
    REQUIRE(m.data[1][0] == 5.0f);
    REQUIRE(m.data[1][1] == 6.0f);
    REQUIRE(m.data[1][2] == 7.0f);
    REQUIRE(m.data[1][3] == 8.0f);
    REQUIRE(m.data[2][0] == 9.0f);
    REQUIRE(m.data[2][1] == 10.0f);
    REQUIRE(m.data[2][2] == 11.0f);
    REQUIRE(m.data[2][3] == 12.0f);
    REQUIRE(m.data[3][0] == 13.0f);
    REQUIRE(m.data[3][1] == 14.0f);
    REQUIRE(m.data[3][2] == 15.0f);
    REQUIRE(m.data[3][3] == 16.0f);
}

TEST_CASE("Matrix4x4 copy constructor", "[Matrix4x4][constructor]")
{
    Matrix4x4 a(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    Matrix4x4 b(a);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(b.data[i][j] == a.data[i][j]);
}

// ============ SetIdentity / SetZero Tests ============

TEST_CASE("Matrix4x4 SetIdentity", "[Matrix4x4][identity]")
{
    Matrix4x4 m;
    m.SetIdentity();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
                REQUIRE(m.data[i][j] == 1.0f);
            else
                REQUIRE(m.data[i][j] == 0.0f);
        }
    }
}

TEST_CASE("Matrix4x4 SetZero", "[Matrix4x4][zero]")
{
    Matrix4x4 m;
    m.SetIdentity();
    m.SetZero();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(m.data[i][j] == 0.0f);
}

// ============ GetTranspose Tests ============

TEST_CASE("Matrix4x4 GetTranspose", "[Matrix4x4][transpose]")
{
    Matrix4x4 m(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    Matrix4x4 t = m.GetTranspose();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(t.data[i][j] == m.data[j][i]);
}

TEST_CASE("Matrix4x4 transpose of identity is identity", "[Matrix4x4][transpose]")
{
    Matrix4x4 m = Matrix4x4::IDENTITY;
    Matrix4x4 t = m.GetTranspose();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(t.data[i][j] == m.data[i][j]);
}

// ============ GetInverse Tests ============

TEST_CASE("Matrix4x4 GetInverse of identity", "[Matrix4x4][inverse]")
{
    Matrix4x4 m = Matrix4x4::IDENTITY;
    Matrix4x4 inv = m.GetInverse();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(m.data[i][j], 1e-5f));
}

TEST_CASE("Matrix4x4 GetInverse of translation", "[Matrix4x4][inverse]")
{
    Matrix4x4 translation(
        1.0f, 0.0f, 0.0f, 3.0f,
        0.0f, 1.0f, 0.0f, 4.0f,
        0.0f, 0.0f, 1.0f, 5.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    Matrix4x4 inv = translation.GetInverse();
    Matrix4x4 expected(
        1.0f, 0.0f, 0.0f, -3.0f,
        0.0f, 1.0f, 0.0f, -4.0f,
        0.0f, 0.0f, 1.0f, -5.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(expected.data[i][j], 1e-5f));
}

TEST_CASE("Matrix4x4 GetInverse of rotation (M^-1 == M^T)", "[Matrix4x4][inverse]")
{
    // Rotation around Z axis by 90 degrees (PI/2 radians)
    Float c = 0.0f;  // cos(90°) = 0
    Float s = 1.0f;  // sin(90°) = 1
    Matrix4x4 rotation(
        c,  -s,  0.0f, 0.0f,
        s,   c,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    Matrix4x4 inv = rotation.GetInverse();
    Matrix4x4 trans = rotation.GetTranspose();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(trans.data[i][j], 1e-5f));
}

TEST_CASE("Matrix4x4 GetInverse singular matrix returns identity", "[Matrix4x4][inverse]")
{
    Matrix4x4 singular(
        1.0f, 2.0f, 3.0f, 4.0f,
        2.0f, 4.0f, 6.0f, 8.0f,
        3.0f, 6.0f, 9.0f, 12.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
    Matrix4x4 inv = singular.GetInverse();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(Matrix4x4::IDENTITY.data[i][j], 1e-5f));
}

TEST_CASE("Matrix4x4 inverse verification (M * M^-1 == I)", "[Matrix4x4][inverse]")
{
    Matrix4x4 m(
        2.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 3.0f, 0.0f, 2.0f,
        0.0f, 0.0f, 4.0f, 3.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    Matrix4x4 inv = m.GetInverse();
    Matrix4x4 product = m * inv;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(product.data[i][j], WithinAbs(Matrix4x4::IDENTITY.data[i][j], 1e-4f));
}

// ============ Matrix * Matrix Tests ============

TEST_CASE("Matrix4x4 multiplication by identity", "[Matrix4x4][multiplication]")
{
    Matrix4x4 m(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    Matrix4x4 result = m * Matrix4x4::IDENTITY;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(result.data[i][j], WithinAbs(m.data[i][j], 1e-5f));
}

TEST_CASE("Matrix4x4 translation * inverse = identity", "[Matrix4x4][multiplication]")
{
    Matrix4x4 translation(
        1.0f, 0.0f, 0.0f, 5.0f,
        0.0f, 1.0f, 0.0f, 10.0f,
        0.0f, 0.0f, 1.0f, 15.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    Matrix4x4 inv = translation.GetInverse();
    Matrix4x4 product = translation * inv;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE_THAT(product.data[i][j], WithinAbs(Matrix4x4::IDENTITY.data[i][j], 1e-5f));
}

// ============ Matrix * Vector4 Tests ============

TEST_CASE("Matrix4x4 identity * Vector4", "[Matrix4x4][vector]")
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 result = Matrix4x4::IDENTITY * v;
    REQUIRE_THAT(result.x, WithinAbs(v.x, 1e-5f));
    REQUIRE_THAT(result.y, WithinAbs(v.y, 1e-5f));
    REQUIRE_THAT(result.z, WithinAbs(v.z, 1e-5f));
    REQUIRE_THAT(result.w, WithinAbs(v.w, 1e-5f));
}

TEST_CASE("Matrix4x4 translation * Vector4", "[Matrix4x4][vector]")
{
    Matrix4x4 translation(
        1.0f, 0.0f, 0.0f, 3.0f,
        0.0f, 1.0f, 0.0f, 4.0f,
        0.0f, 0.0f, 1.0f, 5.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    Vector4 v(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 result = translation * v;
    REQUIRE_THAT(result.x, WithinAbs(4.0f, 1e-5f));
    REQUIRE_THAT(result.y, WithinAbs(5.0f, 1e-5f));
    REQUIRE_THAT(result.z, WithinAbs(6.0f, 1e-5f));
    REQUIRE_THAT(result.w, WithinAbs(1.0f, 1e-5f));
}

// ============ Projection Matrix Tests ============

TEST_CASE("Matrix4x4 Orthographic produces invertible matrix", "[Matrix4x4][projection]")
{
    // Orthographic(l, r, t, b, f, n) where f and n are coordinates (negative)
    Matrix4x4 ortho = Matrix4x4::Orthographic(-1.0f, 1.0f, 1.0f, -1.0f, -100.0f, -1.0f);
    // Verify not identity and not zero
    bool is_identity = true;
    bool is_zero = true;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (ortho.data[i][j] != Matrix4x4::IDENTITY.data[i][j]) is_identity = false;
            if (ortho.data[i][j] != 0.0f) is_zero = false;
        }
    }
    REQUIRE_FALSE(is_identity);
    REQUIRE_FALSE(is_zero);
    // Should be invertible (not singular)
    Matrix4x4 inv = ortho.GetInverse();
    bool inv_is_identity = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (inv.data[i][j] != Matrix4x4::IDENTITY.data[i][j]) inv_is_identity = false;
    REQUIRE_FALSE(inv_is_identity);
}

TEST_CASE("Matrix4x4 Perspective produces invertible matrix", "[Matrix4x4][projection]")
{
    // fov in radians, f and n are coordinates (negative)
    Float fov = MathUtil::DegreesToRadians(90.0f);
    Matrix4x4 persp = Matrix4x4::Perspective(fov, 16.0f / 9.0f, -100.0f, -1.0f);
    // Verify not identity and not zero
    bool is_zero = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (persp.data[i][j] != 0.0f) is_zero = false;
    REQUIRE_FALSE(is_zero);
    // Should be invertible
    Matrix4x4 inv = persp.GetInverse();
    bool inv_is_identity = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (inv.data[i][j] != Matrix4x4::IDENTITY.data[i][j]) inv_is_identity = false;
    REQUIRE_FALSE(inv_is_identity);
}

// ============ GetRow / GetColumn / SetRow / SetColumn Tests ============

TEST_CASE("Matrix4x4 GetRow", "[Matrix4x4][rowcol]")
{
    Matrix4x4 m(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    Vector4 row0 = m.GetRow(0);
    REQUIRE(row0.x == 1.0f);
    REQUIRE(row0.y == 2.0f);
    REQUIRE(row0.z == 3.0f);
    REQUIRE(row0.w == 4.0f);
}

TEST_CASE("Matrix4x4 GetColumn", "[Matrix4x4][rowcol]")
{
    Matrix4x4 m(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    Vector4 col1 = m.GetColumn(1);
    REQUIRE(col1.x == 2.0f);
    REQUIRE(col1.y == 6.0f);
    REQUIRE(col1.z == 10.0f);
    REQUIRE(col1.w == 14.0f);
}

TEST_CASE("Matrix4x4 SetRow", "[Matrix4x4][rowcol]")
{
    Matrix4x4 m = Matrix4x4::IDENTITY;
    Vector4 new_row(10.0f, 20.0f, 30.0f, 40.0f);
    m.SetRow(1, new_row);
    REQUIRE(m.data[1][0] == 10.0f);
    REQUIRE(m.data[1][1] == 20.0f);
    REQUIRE(m.data[1][2] == 30.0f);
    REQUIRE(m.data[1][3] == 40.0f);
}

TEST_CASE("Matrix4x4 SetColumn", "[Matrix4x4][rowcol]")
{
    Matrix4x4 m = Matrix4x4::IDENTITY;
    Vector4 new_col(10.0f, 20.0f, 30.0f, 40.0f);
    m.SetColumn(2, new_col);
    REQUIRE(m.data[0][2] == 10.0f);
    REQUIRE(m.data[1][2] == 20.0f);
    REQUIRE(m.data[2][2] == 30.0f);
    REQUIRE(m.data[3][2] == 40.0f);
}

// ============ Compound Operator Tests ============

TEST_CASE("Matrix4x4 +=", "[Matrix4x4][operator]")
{
    Matrix4x4 a = Matrix4x4::IDENTITY;
    a += Matrix4x4::IDENTITY;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(a.data[i][j] == (i == j ? 2.0f : 0.0f));
}

TEST_CASE("Matrix4x4 -=", "[Matrix4x4][operator]")
{
    Matrix4x4 a = Matrix4x4::IDENTITY;
    a -= Matrix4x4::IDENTITY;
    Matrix4x4 zero = a;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(zero.data[i][j] == 0.0f);
}

TEST_CASE("Matrix4x4 *= (matrix)", "[Matrix4x4][operator]")
{
    Matrix4x4 a = Matrix4x4::IDENTITY;
    Matrix4x4 b(
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 2.0f);
    a *= b;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            REQUIRE(a.data[i][j] == (i == j ? 2.0f : 0.0f));
}

// ============ Static Constants Tests ============

TEST_CASE("Matrix4x4 static constants", "[Matrix4x4][constants]")
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            REQUIRE(Matrix4x4::ZERO.data[i][j] == 0.0f);
            REQUIRE(Matrix4x4::IDENTITY.data[i][j] == (i == j ? 1.0f : 0.0f));
        }
    }
}
