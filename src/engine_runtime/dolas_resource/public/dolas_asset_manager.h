#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <concepts>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "dolas_asset_load_result.h"
#include "dolas_asset_path.h"
#include "dolas_base.h"
#include "dolas_paths.h"
#include "rsd_field.h"

namespace Dolas
{
    // Describes a generated RSD type that AssetManager can load and cache.
    template<class TRsd>
    concept RsdType = std::default_initializable<TRsd>
        && std::move_constructible<TRsd>
        && requires
        {
            { TRsd::kFileSuffix } -> std::convertible_to<const char*>;
            { TRsd::kFields.data() } -> std::same_as<const RsdFieldDesc*>;
            { TRsd::kFields.size() } -> std::same_as<std::size_t>;
        };

    // 按类型自动创建/缓存的 RSD 容器（type-erasure），避免新增资产类型时还要改 AssetManager 成员。
    struct IRsdCache
    {
        virtual ~IRsdCache() = default;
        virtual void Clear() = 0;
    };

    template<RsdType TRsd>
    struct RsdCache final : IRsdCache
    {
        std::unordered_map<AssetPath, TRsd, AssetPathHash> map;
        void Clear() override { map.clear(); }
    };

    // 旧的 CameraAsset / SceneAsset 已废弃：渲染与管理全部改为直接依赖 CameraRSD / SceneRSD

    class AssetManager
    {
    public:
        AssetManager();
        ~AssetManager();

        Bool Initialize();
        Bool Clear(); 

        template<RsdType TRsd>
        [[nodiscard]] AssetLoadResult<TRsd> LoadRsdAsset(const AssetPath& asset_path);

    private:
        // Keeps the XML implementation out of the public template interface.
        AssetLoadError LoadAndParseRsdFile(
            const std::string& file_path,
            void* out_base,
            const RsdFieldDesc* fields,
            std::size_t field_count);

        template<RsdType TRsd>
        RsdCache<TRsd>& GetTypedCache()
        {
            const std::type_index k(typeid(TRsd));
            auto& ptr = m_rsd_caches[k];
            if (!ptr) ptr = std::make_unique<RsdCache<TRsd>>();
            return *static_cast<RsdCache<TRsd>*>(ptr.get());
        }

        std::unordered_map<std::type_index, std::unique_ptr<IRsdCache>> m_rsd_caches;
    };

    template<RsdType TRsd>
    AssetLoadResult<TRsd> AssetManager::LoadRsdAsset(const AssetPath& asset_path)
    {
        if (!asset_path.GetRelativePath().ends_with(TRsd::kFileSuffix))
        {
            return {nullptr, AssetLoadError::FileSuffixMismatch};
        }

        const auto file_path = PathUtils::ResolveAssetPath(asset_path);
        if (!file_path)
        {
            return {nullptr, AssetLoadError::PathResolutionFailed};
        }

        auto& cache = GetTypedCache<TRsd>().map;
        const auto it = cache.find(asset_path);

        if (it != cache.end())
        {
            return {&it->second, AssetLoadError::None};
        }

        TRsd value{};
        const AssetLoadError error = LoadAndParseRsdFile(
            file_path->string(),
            &value,
            TRsd::kFields.data(),
            TRsd::kFields.size());
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
