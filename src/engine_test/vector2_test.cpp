#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ Constructor Tests ============

TEST_CASE("Vector2 default constructor", "[Vector2][constructor]")
{
    Vector2 v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
}

TEST_CASE("Vector2 parameterized constructor", "[Vector2][constructor]")
{
    Vector2 v(3.0f, 4.0f);
    REQUIRE(v.x == 3.0f);
    REQUIRE(v.y == 4.0f);
}

TEST_CASE("Vector2 copy constructor", "[Vector2][constructor]")
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(a);
    REQUIRE(b.x == a.x);
    REQUIRE(b.y == a.y);
}

// ============ Math Methods Tests ============

TEST_CASE("Vector2 Length", "[Vector2][math]")
{
    Vector2 v(3.0f, 4.0f);
    REQUIRE_THAT(v.Length(), WithinAbs(5.0f, 1e-5f));
}

TEST_CASE("Vector2 Dot", "[Vector2][math]")
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    // dot = 1*3 + 2*4 = 11
    REQUIRE_THAT(a.Dot(b), WithinAbs(11.0f, 1e-5f));
}

TEST_CASE("Vector2 Normalized", "[Vector2][math]")
{
    Vector2 v(3.0f, 4.0f);
    Vector2 n = v.Normalized();
    REQUIRE_THAT(n.x, WithinAbs(0.6f, 1e-5f));
    REQUIRE_THAT(n.y, WithinAbs(0.8f, 1e-5f));
    REQUIRE_THAT(n.Length(), WithinAbs(1.0f, 1e-5f));
}

// ============ Arithmetic Operators Tests ============

TEST_CASE("Vector2 addition", "[Vector2][operator]")
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);
    Vector2 c = a + b;
    REQUIRE(c.x == 4.0f);
    REQUIRE(c.y == 6.0f);
}

TEST_CASE("Vector2 subtraction", "[Vector2][operator]")
{
    Vector2 a(5.0f, 7.0f);
    Vector2 b(2.0f, 3.0f);
    Vector2 c = a - b;
    REQUIRE(c.x == 3.0f);
    REQUIRE(c.y == 4.0f);
}

TEST_CASE("Vector2 component-wise multiplication", "[Vector2][operator]")
{
    Vector2 a(2.0f, 3.0f);
    Vector2 b(4.0f, 5.0f);
    Vector2 c = a * b;
    REQUIRE(c.x == 8.0f);
    REQUIRE(c.y == 15.0f);
}

TEST_CASE("Vector2 component-wise division", "[Vector2][operator]")
{
    Vector2 a(10.0f, 20.0f);
    Vector2 b(2.0f, 5.0f);
    Vector2 c = a / b;
    REQUIRE(c.x == 5.0f);
    REQUIRE(c.y == 4.0f);
}

TEST_CASE("Vector2 scalar multiplication", "[Vector2][operator]")
{
    Vector2 v(3.0f, 4.0f);
    Vector2 result = v * 2.0f;
    REQUIRE(result.x == 6.0f);
    REQUIRE(result.y == 8.0f);
}

TEST_CASE("Vector2 scalar division", "[Vector2][operator]")
{
    Vector2 v(6.0f, 8.0f);
    Vector2 result = v / 2.0f;
    REQUIRE(result.x == 3.0f);
    REQUIRE(result.y == 4.0f);
}

// ============ Compound Assignment Operators Tests ============

TEST_CASE("Vector2 +=", "[Vector2][operator]")
{
    Vector2 a(1.0f, 2.0f);
    a += Vector2(3.0f, 4.0f);
    REQUIRE(a.x == 4.0f);
    REQUIRE(a.y == 6.0f);
}

TEST_CASE("Vector2 -=", "[Vector2][operator]")
{
    Vector2 a(5.0f, 7.0f);
    a -= Vector2(2.0f, 3.0f);
    REQUIRE(a.x == 3.0f);
    REQUIRE(a.y == 4.0f);
}

TEST_CASE("Vector2 *= (vector)", "[Vector2][operator]")
{
    Vector2 a(2.0f, 3.0f);
    a *= Vector2(4.0f, 5.0f);
    REQUIRE(a.x == 8.0f);
    REQUIRE(a.y == 15.0f);
}

TEST_CASE("Vector2 /= (vector)", "[Vector2][operator]")
{
    Vector2 a(10.0f, 20.0f);
    a /= Vector2(2.0f, 5.0f);
    REQUIRE(a.x == 5.0f);
    REQUIRE(a.y == 4.0f);
}

TEST_CASE("Vector2 *= (scalar)", "[Vector2][operator]")
{
    Vector2 v(3.0f, 4.0f);
    v *= 2.0f;
    REQUIRE(v.x == 6.0f);
    REQUIRE(v.y == 8.0f);
}

TEST_CASE("Vector2 /= (scalar)", "[Vector2][operator]")
{
    Vector2 v(6.0f, 8.0f);
    v /= 2.0f;
    REQUIRE(v.x == 3.0f);
    REQUIRE(v.y == 4.0f);
}

// ============ Static Constants Tests ============

TEST_CASE("Vector2 static constants", "[Vector2][constants]")
{
    REQUIRE(Vector2::ZERO.x == 0.0f);
    REQUIRE(Vector2::ZERO.y == 0.0f);

    REQUIRE(Vector2::UNIT_X.x == 1.0f);
    REQUIRE(Vector2::UNIT_X.y == 0.0f);

    REQUIRE(Vector2::UNIT_Y.x == 0.0f);
    REQUIRE(Vector2::UNIT_Y.y == 1.0f);

    REQUIRE(Vector2::UNIT_X_NEGATIVE.x == -1.0f);
    REQUIRE(Vector2::UNIT_X_NEGATIVE.y == 0.0f);

    REQUIRE(Vector2::UNIT_Y_NEGATIVE.x == 0.0f);
    REQUIRE(Vector2::UNIT_Y_NEGATIVE.y == -1.0f);
}
