#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ Constructor Tests ============

TEST_CASE("Matrix3x3 default constructor", "[Matrix3x3][constructor]")
{
    Matrix3x3 m;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(m.data[i][j] == 0.0f);
}

TEST_CASE("Matrix3x3 9-parameter constructor", "[Matrix3x3][constructor]")
{
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    REQUIRE(m.data[0][0] == 1.0f);
    REQUIRE(m.data[0][1] == 2.0f);
    REQUIRE(m.data[0][2] == 3.0f);
    REQUIRE(m.data[1][0] == 4.0f);
    REQUIRE(m.data[1][1] == 5.0f);
    REQUIRE(m.data[1][2] == 6.0f);
    REQUIRE(m.data[2][0] == 7.0f);
    REQUIRE(m.data[2][1] == 8.0f);
    REQUIRE(m.data[2][2] == 9.0f);
}

TEST_CASE("Matrix3x3 copy constructor", "[Matrix3x3][constructor]")
{
    Matrix3x3 a(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Matrix3x3 b(a);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(b.data[i][j] == a.data[i][j]);
}

// ============ SetIdentity / SetZero Tests ============

TEST_CASE("Matrix3x3 SetIdentity", "[Matrix3x3][identity]")
{
    Matrix3x3 m;
    m.SetIdentity();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i == j)
                REQUIRE(m.data[i][j] == 1.0f);
            else
                REQUIRE(m.data[i][j] == 0.0f);
        }
    }
}

TEST_CASE("Matrix3x3 SetZero", "[Matrix3x3][zero]")
{
    Matrix3x3 m;
    m.SetIdentity();
    m.SetZero();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(m.data[i][j] == 0.0f);
}

// ============ GetTranspose Tests ============

TEST_CASE("Matrix3x3 GetTranspose", "[Matrix3x3][transpose]")
{
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Matrix3x3 t = m.GetTranspose();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(t.data[i][j] == m.data[j][i]);
}

// ============ GetInverse Tests ============

TEST_CASE("Matrix3x3 GetInverse of identity", "[Matrix3x3][inverse]")
{
    Matrix3x3 m = Matrix3x3::IDENTITY;
    Matrix3x3 inv = m.GetInverse();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(m.data[i][j], 1e-5f));
}

TEST_CASE("Matrix3x3 GetInverse of rotation (M^-1 == M^T)", "[Matrix3x3][inverse]")
{
    // Rotation around Z axis by 90 degrees
    Float c = 0.0f;
    Float s = 1.0f;
    Matrix3x3 rotation(
        c,  -s,  0.0f,
        s,   c,  0.0f,
        0.0f, 0.0f, 1.0f);
    Matrix3x3 inv = rotation.GetInverse();
    Matrix3x3 trans = rotation.GetTranspose();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(trans.data[i][j], 1e-5f));
}

TEST_CASE("Matrix3x3 GetInverse of diagonal matrix", "[Matrix3x3][inverse]")
{
    Matrix3x3 m(
        2.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f,
        0.0f, 0.0f, 4.0f);
    Matrix3x3 inv = m.GetInverse();
    REQUIRE_THAT(inv.data[0][0], WithinAbs(0.5f, 1e-5f));
    REQUIRE_THAT(inv.data[1][1], WithinAbs(1.0f / 3.0f, 1e-5f));
    REQUIRE_THAT(inv.data[2][2], WithinAbs(0.25f, 1e-5f));
}

TEST_CASE("Matrix3x3 GetInverse singular matrix returns identity", "[Matrix3x3][inverse]")
{
    Matrix3x3 singular(
        1.0f, 2.0f, 3.0f,
        2.0f, 4.0f, 6.0f,
        0.0f, 0.0f, 0.0f);
    Matrix3x3 inv = singular.GetInverse();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE_THAT(inv.data[i][j], WithinAbs(Matrix3x3::IDENTITY.data[i][j], 1e-5f));
}

// ============ Matrix * Matrix Tests ============

TEST_CASE("Matrix3x3 multiplication by identity", "[Matrix3x3][multiplication]")
{
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Matrix3x3 result = m * Matrix3x3::IDENTITY;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE_THAT(result.data[i][j], WithinAbs(m.data[i][j], 1e-5f));
}

TEST_CASE("Matrix3x3 operator+", "[Matrix3x3][operator]")
{
    Matrix3x3 a(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Matrix3x3 b = a + a;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(b.data[i][j] == 2.0f * a.data[i][j]);
}

// ============ Matrix * Vector3 Tests ============

TEST_CASE("Matrix3x3 identity * Vector3", "[Matrix3x3][vector]")
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    Vector3 result = Matrix3x3::IDENTITY * v;
    REQUIRE_THAT(result.x, WithinAbs(v.x, 1e-5f));
    REQUIRE_THAT(result.y, WithinAbs(v.y, 1e-5f));
    REQUIRE_THAT(result.z, WithinAbs(v.z, 1e-5f));
}

TEST_CASE("Matrix3x3 scale * Vector3", "[Matrix3x3][vector]")
{
    Matrix3x3 scale(
        2.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f,
        0.0f, 0.0f, 4.0f);
    Vector3 v(1.0f, 1.0f, 1.0f);
    Vector3 result = scale * v;
    REQUIRE(result.x == 2.0f);
    REQUIRE(result.y == 3.0f);
    REQUIRE(result.z == 4.0f);
}

// ============ ExpandToMatrix4x4 Tests ============

TEST_CASE("Matrix3x3 ExpandToMatrix4x4", "[Matrix3x3][expand]")
{
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Matrix4x4 expanded = m.ExpandToMatrix4x4();
    REQUIRE(expanded.data[0][0] == 1.0f);
    REQUIRE(expanded.data[0][1] == 2.0f);
    REQUIRE(expanded.data[0][2] == 3.0f);
    REQUIRE(expanded.data[0][3] == 0.0f);
    REQUIRE(expanded.data[1][0] == 4.0f);
    REQUIRE(expanded.data[1][1] == 5.0f);
    REQUIRE(expanded.data[1][2] == 6.0f);
    REQUIRE(expanded.data[1][3] == 0.0f);
    REQUIRE(expanded.data[2][0] == 7.0f);
    REQUIRE(expanded.data[2][1] == 8.0f);
    REQUIRE(expanded.data[2][2] == 9.0f);
    REQUIRE(expanded.data[2][3] == 0.0f);
    REQUIRE(expanded.data[3][0] == 0.0f);
    REQUIRE(expanded.data[3][1] == 0.0f);
    REQUIRE(expanded.data[3][2] == 0.0f);
    REQUIRE(expanded.data[3][3] == 1.0f);
}

// ============ GetRow / GetColumn / SetRow / SetColumn Tests ============

TEST_CASE("Matrix3x3 GetRow", "[Matrix3x3][rowcol]")
{
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Vector3 row1 = m.GetRow(1);
    REQUIRE(row1.x == 4.0f);
    REQUIRE(row1.y == 5.0f);
    REQUIRE(row1.z == 6.0f);
}

TEST_CASE("Matrix3x3 GetColumn", "[Matrix3x3][rowcol]")
{
    Matrix3x3 m(
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
        7.0f, 8.0f, 9.0f);
    Vector3 col2 = m.GetColumn(2);
    REQUIRE(col2.x == 3.0f);
    REQUIRE(col2.y == 6.0f);
    REQUIRE(col2.z == 9.0f);
}

TEST_CASE("Matrix3x3 SetRow", "[Matrix3x3][rowcol]")
{
    Matrix3x3 m = Matrix3x3::IDENTITY;
    Vector3 new_row(10.0f, 20.0f, 30.0f);
    m.SetRow(0, new_row);
    REQUIRE(m.data[0][0] == 10.0f);
    REQUIRE(m.data[0][1] == 20.0f);
    REQUIRE(m.data[0][2] == 30.0f);
}

TEST_CASE("Matrix3x3 SetColumn", "[Matrix3x3][rowcol]")
{
    Matrix3x3 m = Matrix3x3::IDENTITY;
    Vector3 new_col(10.0f, 20.0f, 30.0f);
    m.SetColumn(1, new_col);
    REQUIRE(m.data[0][1] == 10.0f);
    REQUIRE(m.data[1][1] == 20.0f);
    REQUIRE(m.data[2][1] == 30.0f);
}

// ============ Compound Operator Tests ============

TEST_CASE("Matrix3x3 +=", "[Matrix3x3][operator]")
{
    Matrix3x3 a = Matrix3x3::IDENTITY;
    a += Matrix3x3::IDENTITY;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(a.data[i][j] == (i == j ? 2.0f : 0.0f));
}

TEST_CASE("Matrix3x3 -=", "[Matrix3x3][operator]")
{
    Matrix3x3 a = Matrix3x3::IDENTITY;
    a -= Matrix3x3::IDENTITY;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(a.data[i][j] == 0.0f);
}

TEST_CASE("Matrix3x3 *= (matrix)", "[Matrix3x3][operator]")
{
    Matrix3x3 a = Matrix3x3::IDENTITY;
    Matrix3x3 b(
        2.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f,
        0.0f, 0.0f, 2.0f);
    a *= b;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE(a.data[i][j] == (i == j ? 2.0f : 0.0f));
}

// ============ Static Constants Tests ============

TEST_CASE("Matrix3x3 static constants", "[Matrix3x3][constants]")
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            REQUIRE(Matrix3x3::ZERO.data[i][j] == 0.0f);
            REQUIRE(Matrix3x3::IDENTITY.data[i][j] == (i == j ? 1.0f : 0.0f));
        }
    }
}
