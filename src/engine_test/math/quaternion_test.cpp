#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dolas_math.h"

using namespace Dolas;
using Catch::Matchers::WithinAbs;

// ============ Constructor Tests ============

TEST_CASE("Quaternion default constructor", "[Quaternion][constructor]")
{
    Quaternion q;
    REQUIRE(q.w == 1.0f);
    REQUIRE(q.x == 0.0f);
    REQUIRE(q.y == 0.0f);
    REQUIRE(q.z == 0.0f);
}

TEST_CASE("Quaternion wxyz constructor", "[Quaternion][constructor]")
{
    Quaternion q(0.5f, 0.1f, 0.2f, 0.3f);
    REQUIRE(q.w == 0.5f);
    REQUIRE(q.x == 0.1f);
    REQUIRE(q.y == 0.2f);
    REQUIRE(q.z == 0.3f);
}

TEST_CASE("Quaternion axis-angle zero angle produces identity", "[Quaternion][constructor]")
{
    Vector3 axis(1.0f, 0.0f, 0.0f);
    Quaternion q(axis, 0.0f);
    REQUIRE_THAT(q.w, WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(q.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.z, WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("Quaternion axis-angle 90 degrees around X (clockwise-positive)", "[Quaternion][constructor]")
{
    // The engine uses a clockwise-positive convention: half_angle = -0.5 * radians
    // For 90° around X: half_angle = -PI/4
    // w = cos(-PI/4) = sqrt(2)/2, x = sin(-PI/4) = -sqrt(2)/2
    Vector3 axis(1.0f, 0.0f, 0.0f);
    Quaternion q(axis, 90.0f);
    Float sqrt2_2 = 0.70710678f;
    REQUIRE_THAT(q.w, WithinAbs(sqrt2_2, 1e-5f));
    REQUIRE_THAT(q.x, WithinAbs(-sqrt2_2, 1e-5f));
    REQUIRE_THAT(q.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.z, WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("Quaternion axis-angle zero-length axis produces identity", "[Quaternion][constructor]")
{
    Vector3 axis(0.0f, 0.0f, 0.0f);
    Quaternion q(axis, 90.0f);
    REQUIRE_THAT(q.w, WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(q.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.z, WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("Quaternion axis-angle 90 degrees around Y (clockwise-positive)", "[Quaternion][constructor]")
{
    // 90° around Y axis: half_angle = -PI/4
    // w = sqrt(2)/2, y = -sqrt(2)/2 (since axis is Y)
    Vector3 axis(0.0f, 1.0f, 0.0f);
    Quaternion q(axis, 90.0f);
    Float sqrt2_2 = 0.70710678f;
    REQUIRE_THAT(q.w, WithinAbs(sqrt2_2, 1e-5f));
    REQUIRE_THAT(q.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.y, WithinAbs(-sqrt2_2, 1e-5f));
    REQUIRE_THAT(q.z, WithinAbs(0.0f, 1e-5f));
}

TEST_CASE("Quaternion axis-angle 90 degrees around Z (clockwise-positive)", "[Quaternion][constructor]")
{
    // 90° around Z axis: half_angle = -PI/4
    // w = sqrt(2)/2, z = -sqrt(2)/2 (since axis is Z)
    Vector3 axis(0.0f, 0.0f, 1.0f);
    Quaternion q(axis, 90.0f);
    Float sqrt2_2 = 0.70710678f;
    REQUIRE_THAT(q.w, WithinAbs(sqrt2_2, 1e-5f));
    REQUIRE_THAT(q.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(q.z, WithinAbs(-sqrt2_2, 1e-5f));
}

// ============ Static Constants Tests ============

TEST_CASE("Quaternion IDENTITY constant", "[Quaternion][constants]")
{
    REQUIRE_THAT(Quaternion::IDENTITY.w, WithinAbs(1.0f, 1e-5f));
    REQUIRE_THAT(Quaternion::IDENTITY.x, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(Quaternion::IDENTITY.y, WithinAbs(0.0f, 1e-5f));
    REQUIRE_THAT(Quaternion::IDENTITY.z, WithinAbs(0.0f, 1e-5f));
}
