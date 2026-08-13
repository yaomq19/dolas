#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>

#include "dolas_asset_path.h"
#include "dolas_base.h"
#include "dolas_paths.h"
#include "rsd_field.h"

namespace Dolas
{
    // 按类型自动创建/缓存的 RSD 容器（type-erasure），避免新增资产类型时还要改 AssetManager 成员。
    struct IRsdCache
    {
        virtual ~IRsdCache() = default;
        virtual void Clear() = 0;
    };

    template<class TRsd>
    struct RsdCache final : IRsdCache
    {
        std::unordered_map<std::string, TRsd> map;
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

        template<class TRsd>
        [[nodiscard]] const TRsd* GetRsdAsset(const AssetPath& asset_path);

    protected:
        // 只在 cpp 内部实现（避免其他模块感知 XML/tinyxml2）
        bool LoadAndParseRsdFile(const std::string& file_path, void* outBase, const RsdFieldDesc* fields, std::size_t fieldCount);

        template<class TRsd>
        RsdCache<TRsd>& GetTypedCache()
        {
            const std::type_index k(typeid(TRsd));
            auto& ptr = m_rsd_caches[k];
            if (!ptr) ptr = std::make_unique<RsdCache<TRsd>>();
            return *static_cast<RsdCache<TRsd>*>(ptr.get());
        }

        std::unordered_map<std::type_index, std::unique_ptr<IRsdCache>> m_rsd_caches;
    };

    template<class TRsd>
    const TRsd* AssetManager::GetRsdAsset(const AssetPath& asset_path)
    {
        const auto file_path = PathUtils::ResolveAssetPath(asset_path);
        if (!file_path)
        {
            return nullptr;
        }

        std::unordered_map<std::string, TRsd>& cache = GetTypedCache<TRsd>().map;
        const auto it = cache.find(asset_path.GetCanonicalPath());

        if (it != cache.end())
        {
            return &it->second;
        }

        TRsd value{};
        if (!LoadAndParseRsdFile(file_path->string(), &value, TRsd::kFields.data(), TRsd::kFields.size()))
        {
            return nullptr;
        }

        const auto [inserted, was_inserted] = cache.emplace(asset_path.GetCanonicalPath(), std::move(value));
        (void)was_inserted;
        return &inserted->second;
    }

    class AssetBase
    {
        
    };
    
    class XmlAsset : public AssetBase
    {
        
    };
    
    class RawAsset : public AssetBase
    {
        
    };
    
    class RawAssetWithSpecificExtension : public AssetBase
    {
        
    };
    
    class AssetManagerNew
    {
    public:
        AssetManagerNew();
        ~AssetManagerNew();
        const AssetBase* GetAsset(const AssetPath& asset_path);
        
    private:
        Bool LoadAsset(const AssetPath& asset_path);
    private:
        std::unique_ptr<XmlAsset> LoadXmlAsset(const std::string& absolute_path);
        std::unique_ptr<RawAsset> LoadRawAsset(const std::string& absolute_path);
        
    private:
        std::unordered_map<AssetPath, std::unique_ptr<AssetBase>, AssetPathHash> m_assets;
    };
}
#endif // DOLAS_ASSET_MANAGER_H
