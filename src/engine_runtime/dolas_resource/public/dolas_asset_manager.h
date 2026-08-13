#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <concepts>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>

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
        [[nodiscard]] const TRsd* GetRsdAsset(const AssetPath& asset_path);

    protected:
        // 只在 cpp 内部实现（避免其他模块感知 XML/tinyxml2）
        bool LoadAndParseRsdFile(const std::string& file_path, void* outBase, const RsdFieldDesc* fields, std::size_t fieldCount);
        [[nodiscard]] static bool ValidateRsdFileSuffix(const AssetPath& asset_path, std::string_view expected_suffix);

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
    const TRsd* AssetManager::GetRsdAsset(const AssetPath& asset_path)
    {
        if (!ValidateRsdFileSuffix(asset_path, TRsd::kFileSuffix))
        {
            return nullptr;
        }

        const auto file_path = PathUtils::ResolveAssetPath(asset_path);
        if (!file_path)
        {
            return nullptr;
        }

        auto& cache = GetTypedCache<TRsd>().map;
        const auto it = cache.find(asset_path);

        if (it != cache.end())
        {
            return &it->second;
        }

        TRsd value{};
        if (!LoadAndParseRsdFile(file_path->string(), &value, TRsd::kFields.data(), TRsd::kFields.size()))
        {
            return nullptr;
        }

        const auto [inserted, was_inserted] = cache.emplace(asset_path, std::move(value));
        (void)was_inserted;
        return &inserted->second;
    }
}
#endif // DOLAS_ASSET_MANAGER_H
