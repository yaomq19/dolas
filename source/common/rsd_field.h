// Shared RSD reflection metadata (auto-parse support)
#pragma once

#include <cstddef>
#include <cstdint>

namespace tinyxml2 { class XMLElement; }

namespace Dolas {

struct RsdFieldDesc;

// Generic helper used by generated code to parse a nested object from an XML element.
// Implemented in the engine (AssetManager) compilation unit.
bool ParseRsdObjectFromXmlElement(const tinyxml2::XMLElement* root, void* outBase, const RsdFieldDesc* fields, std::size_t fieldCount);

enum class RsdFieldType : std::uint8_t
{
    Unsupported = 0,
    Object,
    String,
    BoolValue,
    IntValue,
    UIntValue,
    FloatValue,
    Vector3,
    Vector4,
    EnumUInt,
    RawReference,
    AssetReference,
    DynArrayObject,
    DynArrayRawReference,
    DynArrayString,
    DynArrayVector3,
    DynArrayVector4,
    DynArrayFloat,
    DynArrayUInt,
    MapStringVector4,
    MapStringString,
    MapStringRawReference,
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
    const RsdFieldDesc* objectFields; // for Object / DynArrayObject element
    std::size_t objectFieldCount;
    bool (*dynArrayObjectParse)(void* vecMember, const tinyxml2::XMLElement* container); // for DynArrayObject
};

} // namespace Dolas


