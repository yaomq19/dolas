#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "dolas_asset_path.h"
#include "dolas_base.h"
#include "dolas_paths.h"

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

    // Identifies the stage at which an asset load failed.
    enum class AssetLoadError
    {
        None,
        FileSuffixMismatch,
        PathResolutionFailed,
        FileReadFailed,
        JsonParseFailed,
        AssetTypeNotRegistered,
        AssetMetadataInvalid,
        AssetVersionUnsupported,
        AssetFieldParseFailed,
        AssetValidationFailed,
    };

    // Returns a stable, human-readable name suitable for diagnostics.
    [[nodiscard]] constexpr const char* GetAssetLoadErrorName(AssetLoadError error) noexcept
    {
        switch (error)
        {
        case AssetLoadError::None:
            return "none";
        case AssetLoadError::FileSuffixMismatch:
            return "file suffix mismatch";
        case AssetLoadError::PathResolutionFailed:
            return "path resolution failed";
        case AssetLoadError::FileReadFailed:
            return "file read failed";
        case AssetLoadError::JsonParseFailed:
            return "JSON parse failed";
        case AssetLoadError::AssetTypeNotRegistered:
            return "asset type not registered";
        case AssetLoadError::AssetMetadataInvalid:
            return "asset metadata invalid";
        case AssetLoadError::AssetVersionUnsupported:
            return "asset version unsupported";
        case AssetLoadError::AssetFieldParseFailed:
            return "asset field parse failed";
        case AssetLoadError::AssetValidationFailed:
            return "asset validation failed";
        }

        // MSVC cannot prove the switch exhaustive (C4715) without this.
        return "unknown asset load error";
    }

    // Holds either a cached asset pointer or the reason loading failed.
    // The pointer remains valid until its AssetManager is cleared or destroyed.
    template<class TAsset>
    class AssetLoadResult final
    {
    public:
        [[nodiscard]] bool HasValue() const noexcept
        {
            return m_asset != nullptr;
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return HasValue();
        }

        [[nodiscard]] const TAsset* GetAsset() const noexcept
        {
            return m_asset;
        }

        [[nodiscard]] AssetLoadError GetError() const noexcept
        {
            return m_error;
        }

    private:
        friend class AssetManager;

        AssetLoadResult(const TAsset* asset, AssetLoadError error) noexcept
            : m_asset{asset}
            , m_error{error}
        {
        }

        const TAsset* m_asset = nullptr;
        AssetLoadError m_error = AssetLoadError::None;
    };

    // Type-erased cache base; concrete maps still retain their asset type.
    struct IAssetCache
    {
        virtual ~IAssetCache() = default;
    };

    template<AssetDescription TAsset>
    struct AssetCache final : IAssetCache
    {
        std::unordered_map<AssetPath, TAsset, AssetPathHash> map;
    };

    class AssetManager
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        Bool Initialize();
        Bool Clear();

        // Loads a registered C++ asset description and caches it by canonical path.
        template<AssetDescription TAsset>
        [[nodiscard]] AssetLoadResult<TAsset> LoadAsset(const AssetPath& asset_path);

    private:
        using AssetFileLoader = AssetLoadError (*)(const std::string&, void*);

        // Keeps file-format and type-erasure details out of the public template interface.
        AssetLoadError LoadAndParseAssetFile(
            std::string_view type_id,
            const std::string& file_path,
            void* output_asset) const;

        template<AssetDescription TAsset>
        AssetCache<TAsset>& GetTypedCache()
        {
            const std::type_index asset_type{typeid(TAsset)};
            auto& cache = m_asset_caches[asset_type];
            if (!cache)
            {
                cache = std::make_unique<AssetCache<TAsset>>();
            }
            return *static_cast<AssetCache<TAsset>*>(cache.get());
        }

        std::unordered_map<std::string, AssetFileLoader> m_asset_loaders;
        std::unordered_map<std::type_index, std::unique_ptr<IAssetCache>> m_asset_caches;
    };

    template<AssetDescription TAsset>
    AssetLoadResult<TAsset> AssetManager::LoadAsset(const AssetPath& asset_path)
    {
        if (!asset_path.GetRelativePath().ends_with(TAsset::kFileSuffix))
        {
            return {nullptr, AssetLoadError::FileSuffixMismatch};
        }

        const auto file_path = PathUtils::ResolveAssetPath(asset_path);
        if (!file_path)
        {
            return {nullptr, AssetLoadError::PathResolutionFailed};
        }

        auto& cache = GetTypedCache<TAsset>().map;
        const auto it = cache.find(asset_path);

        if (it != cache.end())
        {
            return {&it->second, AssetLoadError::None};
        }

        TAsset value{};
        const AssetLoadError error = LoadAndParseAssetFile(
            TAsset::kTypeId,
            file_path->string(),
            &value);
        if (error != AssetLoadError::None)
        {
            return {nullptr, error};
        }

        const auto [inserted, was_inserted] = cache.emplace(asset_path, std::move(value));
        (void)was_inserted;
        return {&inserted->second, AssetLoadError::None};
    }
}
#endif // DOLAS_ASSET_MANAGER_H
