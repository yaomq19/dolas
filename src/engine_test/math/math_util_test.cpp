#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ DegreesToRadians / RadiansToDegrees Tests ============

TEST_CASE("MathUtil DegreesToRadians known values", "[MathUtil][conversion]")
{
    REQUIRE_THAT(MathUtil::DegreesToRadians(0.0f), WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(MathUtil::DegreesToRadians(180.0f), WithinAbs(MathUtil::PI, 1e-5f));
    REQUIRE_THAT(MathUtil::DegreesToRadians(360.0f), WithinAbs(2.0f * MathUtil::PI, 1e-5f));
    REQUIRE_THAT(MathUtil::DegreesToRadians(90.0f), WithinAbs(MathUtil::PI / 2.0f, 1e-5f));
}

TEST_CASE("MathUtil RadiansToDegrees known values", "[MathUtil][conversion]")
{
    REQUIRE_THAT(MathUtil::RadiansToDegrees(0.0f), WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(MathUtil::RadiansToDegrees(MathUtil::PI), WithinAbs(180.0f, 1e-5f));
    REQUIRE_THAT(MathUtil::RadiansToDegrees(2.0f * MathUtil::PI), WithinAbs(360.0f, 1e-5f));
    REQUIRE_THAT(MathUtil::RadiansToDegrees(MathUtil::PI / 2.0f), WithinAbs(90.0f, 1e-5f));
}

TEST_CASE("MathUtil DegreesToRadians round-trip", "[MathUtil][conversion]")
{
    Float degrees = 57.0f;
    Float radians = MathUtil::DegreesToRadians(degrees);
    Float back = MathUtil::RadiansToDegrees(radians);
    REQUIRE_THAT(back, WithinAbs(degrees, 1e-5f));
}

// ============ Rotate Tests ============

TEST_CASE("MathUtil Rotate zero angle returns identity", "[MathUtil][rotate]")
{
    Vector3 axis(1.0f, 0.0f, 0.0f);
    Matrix3x3 r = MathUtil::Rotate(axis, 0.0f);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE_THAT(r.data[i][j], WithinAbs(Matrix3x3::IDENTITY.data[i][j], 1e-5f));
}

TEST_CASE("MathUtil Rotate around X axis 90 degrees", "[MathUtil][rotate]")
{
    // Rotate PI/2 around X: (0,1,0) -> (0,0,1)
    Vector3 axis(1.0f, 0.0f, 0.0f);
    Matrix3x3 r = MathUtil::Rotate(axis, MathUtil::PI / 2.0f);
    Vector3 v(0.0f, 1.0f, 0.0f);
    Vector3 result = r * v;
    REQUIRE_THAT(result.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(result.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(result.z, WithinAbs(1.0f, 1e-5f));
}

TEST_CASE("MathUtil Rotate around Y axis 90 degrees", "[MathUtil][rotate]")
{
    // Rotate PI/2 around Y: (1,0,0) -> (0,0,-1)
    Vector3 axis(0.0f, 1.0f, 0.0f);
    Matrix3x3 r = MathUtil::Rotate(axis, MathUtil::PI / 2.0f);
    Vector3 v(1.0f, 0.0f, 0.0f);
    Vector3 result = r * v;
    REQUIRE_THAT(result.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(result.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(result.z, WithinAbs(-1.0f, 1e-5f));
}

TEST_CASE("MathUtil Rotate preserves vector length", "[MathUtil][rotate]")
{
    Vector3 axis(1.0f, 1.0f, 1.0f);
    Matrix3x3 r = MathUtil::Rotate(axis, 1.0f);
    Vector3 v(3.0f, 4.0f, 5.0f);
    Vector3 result = r * v;
    REQUIRE_THAT(result.Length(), WithinAbs(v.Length(), 1e-4f));
}

TEST_CASE("MathUtil Rotate produces orthogonal matrix", "[MathUtil][rotate]")
{
    Vector3 axis(0.0f, 0.0f, 1.0f);
    Matrix3x3 r = MathUtil::Rotate(axis, MathUtil::PI / 3.0f);
    // For orthogonal matrix: R * R^T = I
    Matrix3x3 rt = r.GetTranspose();
    Matrix3x3 product = r * rt;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            REQUIRE_THAT(product.data[i][j], WithinAbs(Matrix3x3::IDENTITY.data[i][j], 1e-5f));
}

// ============ PI Constant Test ============

TEST_CASE("MathUtil PI constant", "[MathUtil][constants]")
{
    REQUIRE(MathUtil::PI > 3.14f);
    REQUIRE(MathUtil::PI < 3.142f);
}
