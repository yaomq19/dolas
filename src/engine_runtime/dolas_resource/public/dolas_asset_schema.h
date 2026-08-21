#ifndef DOLAS_ASSET_SCHEMA_H
#define DOLAS_ASSET_SCHEMA_H

#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace Dolas
{
    // A root asset is a value type with stable serialized identity.
    // Field layout is reflected by reflect-cpp directly from the C++ struct.
    template<class TAsset>
    concept AssetDescription = std::default_initializable<TAsset>
        && std::move_constructible<TAsset>
        && requires
        {
            { TAsset::kTypeId } -> std::convertible_to<std::string_view>;
            { TAsset::kFileSuffix } -> std::convertible_to<std::string_view>;
            { TAsset::kSchemaVersion } -> std::convertible_to<std::uint32_t>;
        };
}

#endif // DOLAS_ASSET_SCHEMA_H
