// Shared RSD reflection metadata (auto-parse support)
#pragma once

#include <cstddef>
#include <cstdint>

namespace Dolas {

enum class RsdFieldType : std::uint8_t
{
    Unsupported = 0,
    String,
    BoolValue,
    IntValue,
    UIntValue,
    FloatValue,
    Vector3,
    Vector4,
    EnumUInt,
    DynArrayXml,
    DynArrayFloat,
    DynArrayUInt,
    MapStringVector4,
    MapStringString,
    MapStringFloat
};

// For enum fields, the generic loader needs a mapping from text -> numeric value.
// - name: canonical enumerator name (e.g. "Perspective")
// - display: optional UI label (e.g. "透视")
// - alias: optional alternate token accepted during parsing (e.g. "perspective")
// - value: underlying numeric value (UInt)
struct RsdEnumItemDesc
{
    const char* name;
    const char* display;
    const char* alias;
    std::uint64_t value;
};

struct RsdFieldDesc
{
    const char* name;
    RsdFieldType type;
    std::size_t offset;
    const RsdEnumItemDesc* enumItems; // nullptr unless type==EnumUInt
    std::size_t enumItemCount;
};

} // namespace Dolas


