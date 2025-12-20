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
    DynArrayXml,
    DynArrayFloat,
    DynArrayUInt,
    MapStringVector4,
    MapStringString,
    MapStringFloat
};

struct RsdFieldDesc
{
    const char* name;
    RsdFieldType type;
    std::size_t offset;
};

} // namespace Dolas


