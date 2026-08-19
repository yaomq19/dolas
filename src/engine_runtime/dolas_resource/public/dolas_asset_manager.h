#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "dolas_asset_load_result.h"
#include "dolas_asset_path.h"
#include "dolas_asset_schema.h"
#include "dolas_base.h"
#include "dolas_paths.h"

namespace Dolas
{
    // Type-erased cache interface; concrete maps still retain their asset type.
    struct IAssetCache
    {
        virtual ~IAssetCache() = default;
        virtual void Clear() = 0;
    };

    template<AssetDescription TAsset>
    struct AssetCache final : IAssetCache
    {
        std::unordered_map<AssetPath, TAsset, AssetPathHash> map;
        void Clear() override { map.clear(); }
    };

    class AssetManager
    {
    public:
        AssetManager();
        ~AssetManager();

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
