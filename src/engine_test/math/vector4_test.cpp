#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ Constructor Tests ============

TEST_CASE("Vector4 default constructor", "[Vector4][constructor]")
{
    Vector4 v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
    REQUIRE(v.z == 0.0f);
    REQUIRE(v.w == 0.0f);
}

TEST_CASE("Vector4 parameterized constructor", "[Vector4][constructor]")
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
    REQUIRE(v.w == 4.0f);
}

TEST_CASE("Vector4 Vector3+w constructor", "[Vector4][constructor]")
{
    Vector3 v3(1.0f, 2.0f, 3.0f);
    Vector4 v(v3, 4.0f);
    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
    REQUIRE(v.w == 4.0f);
}

TEST_CASE("Vector4 copy constructor", "[Vector4][constructor]")
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(a);
    REQUIRE(b.x == a.x);
    REQUIRE(b.y == a.y);
    REQUIRE(b.z == a.z);
    REQUIRE(b.w == a.w);
}

// ============ Math Methods Tests ============

TEST_CASE("Vector4 Length", "[Vector4][math]")
{
    Vector4 v(2.0f, 3.0f, 6.0f, 0.0f);
    // sqrt(4 + 9 + 36 + 0) = sqrt(49) = 7
    REQUIRE_THAT(v.Length(), WithinAbs(7.0f, 1e-5f));
}

TEST_CASE("Vector4 LengthSquared", "[Vector4][math]")
{
    Vector4 v(2.0f, 3.0f, 6.0f, 0.0f);
    // 4 + 9 + 36 + 0 = 49
    REQUIRE_THAT(v.LengthSquared(), WithinAbs(49.0f, 1e-5f));
}

TEST_CASE("Vector4 Dot", "[Vector4][math]")
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    // dot = 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
    REQUIRE_THAT(a.Dot(b), WithinAbs(70.0f, 1e-5f));
}

TEST_CASE("Vector4 Cross product", "[Vector4][math]")
{
    // Vector4::Cross: xyz components match Vector3 cross product;
    // w component = w * other.w (non-standard convention).
    Vector4 a(1.0f, 0.0f, 0.0f, 2.0f);
    Vector4 b(0.0f, 1.0f, 0.0f, 3.0f);
    Vector4 c = a.Cross(b);
    // xyz: (1,0,0) x (0,1,0) = (0,0,1)
    REQUIRE_THAT(c.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(c.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(c.z, WithinAbs(1.0f, 1e-5f));
    // w = w * other.w
    REQUIRE_THAT(c.w, WithinAbs(6.0f, 1e-5f));
}

TEST_CASE("Vector4 Normalize (in-place)", "[Vector4][math]")
{
    Vector4 v(3.0f, 0.0f, 4.0f, 0.0f);
    v.Normalize();
    REQUIRE_THAT(v.x, WithinAbs(0.6f, 1e-5f));
    REQUIRE_THAT(v.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(v.z, WithinAbs(0.8f, 1e-5f));
    REQUIRE_THAT(v.w, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(v.Length(), WithinAbs(1.0f, 1e-5f));
}

// ============ Arithmetic Operator Tests ============

TEST_CASE("Vector4 addition", "[Vector4][operator]")
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);
    Vector4 c = a + b;
    REQUIRE(c.x == 6.0f);
    REQUIRE(c.y == 8.0f);
    REQUIRE(c.z == 10.0f);
    REQUIRE(c.w == 12.0f);
}

TEST_CASE("Vector4 subtraction", "[Vector4][operator]")
{
    Vector4 a(10.0f, 8.0f, 6.0f, 4.0f);
    Vector4 b(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 c = a - b;
    REQUIRE(c.x == 9.0f);
    REQUIRE(c.y == 6.0f);
    REQUIRE(c.z == 3.0f);
    REQUIRE(c.w == 0.0f);
}

TEST_CASE("Vector4 component-wise multiplication", "[Vector4][operator]")
{
    Vector4 a(2.0f, 3.0f, 4.0f, 5.0f);
    Vector4 b(6.0f, 7.0f, 8.0f, 9.0f);
    Vector4 c = a * b;
    REQUIRE(c.x == 12.0f);
    REQUIRE(c.y == 21.0f);
    REQUIRE(c.z == 32.0f);
    REQUIRE(c.w == 45.0f);
}

TEST_CASE("Vector4 component-wise division", "[Vector4][operator]")
{
    Vector4 a(20.0f, 15.0f, 12.0f, 9.0f);
    Vector4 b(4.0f, 3.0f, 2.0f, 1.0f);
    Vector4 c = a / b;
    REQUIRE(c.x == 5.0f);
    REQUIRE(c.y == 5.0f);
    REQUIRE(c.z == 6.0f);
    REQUIRE(c.w == 9.0f);
}

TEST_CASE("Vector4 scalar multiplication", "[Vector4][operator]")
{
    Vector4 v(2.0f, 3.0f, 4.0f, 5.0f);
    Vector4 result = v * 3.0f;
    REQUIRE(result.x == 6.0f);
    REQUIRE(result.y == 9.0f);
    REQUIRE(result.z == 12.0f);
    REQUIRE(result.w == 15.0f);
}

TEST_CASE("Vector4 scalar division", "[Vector4][operator]")
{
    Vector4 v(6.0f, 9.0f, 12.0f, 15.0f);
    Vector4 result = v / 3.0f;
    REQUIRE(result.x == 2.0f);
    REQUIRE(result.y == 3.0f);
    REQUIRE(result.z == 4.0f);
    REQUIRE(result.w == 5.0f);
}

// ============ Compound Assignment Tests ============

TEST_CASE("Vector4 +=", "[Vector4][operator]")
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    a += Vector4(5.0f, 6.0f, 7.0f, 8.0f);
    REQUIRE(a.x == 6.0f);
    REQUIRE(a.y == 8.0f);
    REQUIRE(a.z == 10.0f);
    REQUIRE(a.w == 12.0f);
}

TEST_CASE("Vector4 -=", "[Vector4][operator]")
{
    Vector4 a(10.0f, 8.0f, 6.0f, 4.0f);
    a -= Vector4(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(a.x == 9.0f);
    REQUIRE(a.y == 6.0f);
    REQUIRE(a.z == 3.0f);
    REQUIRE(a.w == 0.0f);
}

TEST_CASE("Vector4 *= (vector)", "[Vector4][operator]")
{
    Vector4 a(2.0f, 3.0f, 4.0f, 5.0f);
    a *= Vector4(6.0f, 7.0f, 8.0f, 9.0f);
    REQUIRE(a.x == 12.0f);
    REQUIRE(a.y == 21.0f);
    REQUIRE(a.z == 32.0f);
    REQUIRE(a.w == 45.0f);
}

TEST_CASE("Vector4 /= (vector)", "[Vector4][operator]")
{
    Vector4 a(20.0f, 15.0f, 12.0f, 9.0f);
    a /= Vector4(4.0f, 3.0f, 2.0f, 1.0f);
    REQUIRE(a.x == 5.0f);
    REQUIRE(a.y == 5.0f);
    REQUIRE(a.z == 6.0f);
    REQUIRE(a.w == 9.0f);
}

TEST_CASE("Vector4 *= (scalar)", "[Vector4][operator]")
{
    Vector4 v(2.0f, 3.0f, 4.0f, 5.0f);
    v *= 3.0f;
    REQUIRE(v.x == 6.0f);
    REQUIRE(v.y == 9.0f);
    REQUIRE(v.z == 12.0f);
    REQUIRE(v.w == 15.0f);
}

TEST_CASE("Vector4 /= (scalar)", "[Vector4][operator]")
{
    Vector4 v(6.0f, 9.0f, 12.0f, 15.0f);
    v /= 3.0f;
    REQUIRE(v.x == 2.0f);
    REQUIRE(v.y == 3.0f);
    REQUIRE(v.z == 4.0f);
    REQUIRE(v.w == 5.0f);
}

// ============ Array Access Tests ============

TEST_CASE("Vector4 array access operator []", "[Vector4][operator]")
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(v[0] == 1.0f);
    REQUIRE(v[1] == 2.0f);
    REQUIRE(v[2] == 3.0f);
    REQUIRE(v[3] == 4.0f);
}

TEST_CASE("Vector4 array access operator [] (const)", "[Vector4][operator]")
{
    const Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(v[0] == 1.0f);
    REQUIRE(v[1] == 2.0f);
    REQUIRE(v[2] == 3.0f);
    REQUIRE(v[3] == 4.0f);
}

TEST_CASE("Vector4 array access modification", "[Vector4][operator]")
{
    Vector4 v(1.0f, 2.0f, 3.0f, 4.0f);
    v[0] = 10.0f;
    v[1] = 20.0f;
    v[2] = 30.0f;
    v[3] = 40.0f;
    REQUIRE(v.x == 10.0f);
    REQUIRE(v.y == 20.0f);
    REQUIRE(v.z == 30.0f);
    REQUIRE(v.w == 40.0f);
}

// ============ Static Constants Tests ============

TEST_CASE("Vector4 static constants", "[Vector4][constants]")
{
    REQUIRE(Vector4::ZERO.x == 0.0f);
    REQUIRE(Vector4::ZERO.y == 0.0f);
    REQUIRE(Vector4::ZERO.z == 0.0f);
    REQUIRE(Vector4::ZERO.w == 0.0f);
}
