#include <catch2/catch_test_macros.hpp>
#include "dolas_hash.h"

using namespace Dolas;

// FNV-1a 32-bit test vectors (verified against reference implementation)
// - "" (empty) → 0x811c9dc5 = 2166136261
// - "a"         → 0xe40c292c = 3826002220
// - "hello"     → 0x4f9f2cab = 1335831723
// - "test"      → 0xafd071e5 = 2949673445

TEST_CASE("HashConverter empty string hash equals FNV-1a offset basis", "[HashConverter][fnv1a]")
{
    UInt hash = HashConverter::StringHash("");
    REQUIRE(hash == 2166136261U);
}

TEST_CASE("HashConverter known FNV-1a test vectors", "[HashConverter][fnv1a]")
{
    REQUIRE(HashConverter::StringHash("a") == 3826002220U);
    REQUIRE(HashConverter::StringHash("hello") == 1335831723U);
    REQUIRE(HashConverter::StringHash("test") == 2949673445U);
}

TEST_CASE("HashConverter determinism", "[HashConverter][fnv1a]")
{
    const std::string s = "deterministic_test_string";
    UInt h1 = HashConverter::StringHash(s);
    UInt h2 = HashConverter::StringHash(s);
    REQUIRE(h1 == h2);
}

TEST_CASE("HashConverter different strings produce different hashes", "[HashConverter][fnv1a]")
{
    UInt h1 = HashConverter::StringHash("hello");
    UInt h2 = HashConverter::StringHash("world");
    REQUIRE(h1 != h2);
}

TEST_CASE("HashConverter STRING_ID macro matches StringHash", "[HashConverter][string_id]")
{
    // STRING_ID(x) uses #x (stringized), so STRING_ID(test_macro_value)
    // should equal StringHash("test_macro_value")
    UInt compile_time = STRING_ID(test_macro_value);
    UInt runtime = HashConverter::StringHash("test_macro_value");
    REQUIRE(compile_time == runtime);
}

TEST_CASE("HashConverter STRING_ID with different identifiers", "[HashConverter][string_id]")
{
    UInt id1 = STRING_ID(sphere_render_primitive);
    UInt id2 = HashConverter::StringHash("sphere_render_primitive");
    REQUIRE(id1 == id2);
}

// ============ Debug-only Tests ============

#if defined(DEBUG) || defined(_DEBUG)

TEST_CASE("HashConverter GetString returns original string in debug", "[HashConverter][debug]")
{
    HashConverter::ClearRegistry();
    const std::string original = "debug_test_string";
    UInt hash = HashConverter::StringHash(original);
    REQUIRE(HashConverter::GetString(hash) == original);
}

TEST_CASE("HashConverter GetString returns unknown format for missing hash", "[HashConverter][debug]")
{
    HashConverter::ClearRegistry();
    // Hash that was never registered should return "Unknown[...]" format
    std::string result = HashConverter::GetString(0xDEADBEEF);
    REQUIRE(result.find("Unknown[") == 0);
}

TEST_CASE("HashConverter HasString returns true for registered hash", "[HashConverter][debug]")
{
    HashConverter::ClearRegistry();
    UInt hash = HashConverter::StringHash("has_string_test");
    REQUIRE(HashConverter::HasString(hash) == true);
}

TEST_CASE("HashConverter HasString returns false for unregistered hash", "[HashConverter][debug]")
{
    HashConverter::ClearRegistry();
    REQUIRE(HashConverter::HasString(0xCAFEBABE) == false);
}

TEST_CASE("HashConverter ClearRegistry removes all mappings", "[HashConverter][debug]")
{
    HashConverter::ClearRegistry();
    UInt hash = HashConverter::StringHash("clear_test");
    REQUIRE(HashConverter::HasString(hash) == true);
    HashConverter::ClearRegistry();
    REQUIRE(HashConverter::HasString(hash) == false);
}

#else

TEST_CASE("HashConverter debug-only tests skipped in release", "[HashConverter]")
{
    SUCCEED("Debug-only tests skipped in Release build");
}

#endif
