#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ Vector3 Constructor Tests ============

TEST_CASE("Vector3 default constructor", "[Vector3][constructor]")
{
    Vector3 v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
    REQUIRE(v.z == 0.0f);
}

TEST_CASE("Vector3 parameterized constructor", "[Vector3][constructor]")
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    REQUIRE(v.x == 1.0f);
    REQUIRE(v.y == 2.0f);
    REQUIRE(v.z == 3.0f);
}

TEST_CASE("Vector3 copy constructor", "[Vector3][constructor]")
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(a);
    REQUIRE(b.x == a.x);
    REQUIRE(b.y == a.y);
    REQUIRE(b.z == a.z);
}

// ============ Vector3 Math Methods Tests ============

TEST_CASE("Vector3 Length", "[Vector3][math]")
{
    Vector3 v(2.0f, 3.0f, 6.0f);
    // sqrt(4 + 9 + 36) = sqrt(49) = 7
    REQUIRE_THAT(v.Length(), WithinAbs(7.0f, 1e-5f));
}

TEST_CASE("Vector3 LengthSquared", "[Vector3][math]")
{
    Vector3 v(2.0f, 3.0f, 6.0f);
    // 4 + 9 + 36 = 49
    REQUIRE_THAT(v.LengthSquared(), WithinAbs(49.0f, 1e-5f));
}

TEST_CASE("Vector3 Dot", "[Vector3][math]")
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    // dot = 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    REQUIRE_THAT(a.Dot(b), WithinAbs(32.0f, 1e-5f));
}

TEST_CASE("Vector3 Cross product", "[Vector3][math]")
{
    Vector3 a(1.0f, 0.0f, 0.0f);
    Vector3 b(0.0f, 1.0f, 0.0f);
    Vector3 c = a.Cross(b);
    // i x j = k
    REQUIRE_THAT(c.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(c.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(c.z, WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("Vector3 Normalized", "[Vector3][math]")
{
    Vector3 v(3.0f, 0.0f, 4.0f);
    Vector3 n = v.Normalized();
    REQUIRE_THAT(n.x, WithinAbs(0.6f, 1e-5f));
    REQUIRE_THAT(n.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(n.z, WithinAbs(0.8f, 1e-5f));
    REQUIRE_THAT(n.Length(), WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("Vector3 Normalize (in-place)", "[Vector3][math]")
{
    Vector3 v(3.0f, 0.0f, 4.0f);
    v.Normalize();
    REQUIRE_THAT(v.x, WithinAbs(0.6f, 1e-5f));
    REQUIRE_THAT(v.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(v.z, WithinAbs(0.8f, 1e-5f));
    REQUIRE_THAT(v.Length(), WithinAbs(1.0f, 1e-5f));
}

// ============ Vector3 Arithmetic Operators Tests ============

TEST_CASE("Vector3 addition", "[Vector3][operator]")
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 c = a + b;
    REQUIRE(c.x == 5.0f);
    REQUIRE(c.y == 7.0f);
    REQUIRE(c.z == 9.0f);
}

TEST_CASE("Vector3 subtraction", "[Vector3][operator]")
{
    Vector3 a(10.0f, 8.0f, 6.0f);
    Vector3 b(1.0f, 2.0f, 3.0f);
    Vector3 c = a - b;
    REQUIRE(c.x == 9.0f);
    REQUIRE(c.y == 6.0f);
    REQUIRE(c.z == 3.0f);
}

TEST_CASE("Vector3 negation", "[Vector3][operator]")
{
    Vector3 v(1.0f, -2.0f, 3.0f);
    Vector3 neg = -v;
    REQUIRE(neg.x == -1.0f);
    REQUIRE(neg.y == 2.0f);
    REQUIRE(neg.z == -3.0f);
}

TEST_CASE("Vector3 component-wise multiplication", "[Vector3][operator]")
{
    Vector3 a(2.0f, 3.0f, 4.0f);
    Vector3 b(5.0f, 6.0f, 7.0f);
    Vector3 c = a * b;
    REQUIRE(c.x == 10.0f);
    REQUIRE(c.y == 18.0f);
    REQUIRE(c.z == 28.0f);
}

TEST_CASE("Vector3 component-wise division", "[Vector3][operator]")
{
    Vector3 a(20.0f, 15.0f, 12.0f);
    Vector3 b(4.0f, 3.0f, 2.0f);
    Vector3 c = a / b;
    REQUIRE(c.x == 5.0f);
    REQUIRE(c.y == 5.0f);
    REQUIRE(c.z == 6.0f);
}

TEST_CASE("Vector3 scalar multiplication", "[Vector3][operator]")
{
    Vector3 v(2.0f, 3.0f, 4.0f);
    Vector3 result = v * 3.0f;
    REQUIRE(result.x == 6.0f);
    REQUIRE(result.y == 9.0f);
    REQUIRE(result.z == 12.0f);
}

TEST_CASE("Vector3 scalar division", "[Vector3][operator]")
{
    Vector3 v(6.0f, 9.0f, 12.0f);
    Vector3 result = v / 3.0f;
    REQUIRE(result.x == 2.0f);
    REQUIRE(result.y == 3.0f);
    REQUIRE(result.z == 4.0f);
}

// ============ Vector3 Compound Assignment Tests ============

TEST_CASE("Vector3 +=", "[Vector3][operator]")
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    a += Vector3(4.0f, 5.0f, 6.0f);
    REQUIRE(a.x == 5.0f);
    REQUIRE(a.y == 7.0f);
    REQUIRE(a.z == 9.0f);
}

TEST_CASE("Vector3 -=", "[Vector3][operator]")
{
    Vector3 a(10.0f, 8.0f, 6.0f);
    a -= Vector3(1.0f, 2.0f, 3.0f);
    REQUIRE(a.x == 9.0f);
    REQUIRE(a.y == 6.0f);
    REQUIRE(a.z == 3.0f);
}

TEST_CASE("Vector3 *= (vector)", "[Vector3][operator]")
{
    Vector3 a(2.0f, 3.0f, 4.0f);
    a *= Vector3(5.0f, 6.0f, 7.0f);
    REQUIRE(a.x == 10.0f);
    REQUIRE(a.y == 18.0f);
    REQUIRE(a.z == 28.0f);
}

TEST_CASE("Vector3 /= (vector)", "[Vector3][operator]")
{
    Vector3 a(20.0f, 15.0f, 12.0f);
    a /= Vector3(4.0f, 3.0f, 2.0f);
    REQUIRE(a.x == 5.0f);
    REQUIRE(a.y == 5.0f);
    REQUIRE(a.z == 6.0f);
}

TEST_CASE("Vector3 *= (scalar)", "[Vector3][operator]")
{
    Vector3 v(2.0f, 3.0f, 4.0f);
    v *= 3.0f;
    REQUIRE(v.x == 6.0f);
    REQUIRE(v.y == 9.0f);
    REQUIRE(v.z == 12.0f);
}

TEST_CASE("Vector3 /= (scalar)", "[Vector3][operator]")
{
    Vector3 v(6.0f, 9.0f, 12.0f);
    v /= 3.0f;
    REQUIRE(v.x == 2.0f);
    REQUIRE(v.y == 3.0f);
    REQUIRE(v.z == 4.0f);
}

// ============ Vector3 Array Access Tests ============

TEST_CASE("Vector3 array access operator []", "[Vector3][operator]")
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    REQUIRE(v[0] == 1.0f);
    REQUIRE(v[1] == 2.0f);
    REQUIRE(v[2] == 3.0f);
}

TEST_CASE("Vector3 array access operator [] (const)", "[Vector3][operator]")
{
    const Vector3 v(1.0f, 2.0f, 3.0f);
    REQUIRE(v[0] == 1.0f);
    REQUIRE(v[1] == 2.0f);
    REQUIRE(v[2] == 3.0f);
}

TEST_CASE("Vector3 array access modification", "[Vector3][operator]")
{
    Vector3 v(1.0f, 2.0f, 3.0f);
    v[0] = 10.0f;
    v[1] = 20.0f;
    v[2] = 30.0f;
    REQUIRE(v.x == 10.0f);
    REQUIRE(v.y == 20.0f);
    REQUIRE(v.z == 30.0f);
}

// ============ Vector3 Static Constants Tests ============

TEST_CASE("Vector3 static constants", "[Vector3][constants]")
{
    REQUIRE(Vector3::ZERO.x == 0.0f);
    REQUIRE(Vector3::ZERO.y == 0.0f);
    REQUIRE(Vector3::ZERO.z == 0.0f);

    REQUIRE(Vector3::UNIT_X.x == 1.0f);
    REQUIRE(Vector3::UNIT_X.y == 0.0f);
    REQUIRE(Vector3::UNIT_X.z == 0.0f);

    REQUIRE(Vector3::UNIT_Y.x == 0.0f);
    REQUIRE(Vector3::UNIT_Y.y == 1.0f);
    REQUIRE(Vector3::UNIT_Y.z == 0.0f);

    REQUIRE(Vector3::UNIT_Z.x == 0.0f);
    REQUIRE(Vector3::UNIT_Z.y == 0.0f);
    REQUIRE(Vector3::UNIT_Z.z == 1.0f);

    REQUIRE(Vector3::UNIT_X_NEGATIVE.x == -1.0f);
    REQUIRE(Vector3::UNIT_X_NEGATIVE.y == 0.0f);
    REQUIRE(Vector3::UNIT_X_NEGATIVE.z == 0.0f);

    REQUIRE(Vector3::UNIT_Y_NEGATIVE.x == 0.0f);
    REQUIRE(Vector3::UNIT_Y_NEGATIVE.y == -1.0f);
    REQUIRE(Vector3::UNIT_Y_NEGATIVE.z == 0.0f);

    REQUIRE(Vector3::UNIT_Z_NEGATIVE.x == 0.0f);
    REQUIRE(Vector3::UNIT_Z_NEGATIVE.y == 0.0f);
    REQUIRE(Vector3::UNIT_Z_NEGATIVE.z == -1.0f);
}

// ============ Vector3 Cross Product Advanced Tests ============

TEST_CASE("Vector3 cross product anti-commutativity", "[Vector3][math]")
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 cross1 = a.Cross(b);
    Vector3 cross2 = b.Cross(a);
    Vector3 neg_cross2 = -cross2;
    
    REQUIRE_THAT(cross1.x, WithinAbs(neg_cross2.x, 1e-5f));
    REQUIRE_THAT(cross1.y, WithinAbs(neg_cross2.y, 1e-5f));
    REQUIRE_THAT(cross1.z, WithinAbs(neg_cross2.z, 1e-5f));
}

TEST_CASE("Vector3 cross product perpendicularity", "[Vector3][math]")
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);
    Vector3 c = a.Cross(b);
    
    // c 应该垂直于 a 和 b，所以点积应该为 0
    REQUIRE_THAT(c.Dot(a), WithinAbs(0.0f, 1e-4f));
    REQUIRE_THAT(c.Dot(b), WithinAbs(0.0f, 1e-4f));
}
