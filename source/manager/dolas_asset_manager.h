#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <string>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>
#include "base/dolas_base.h"
#include "base/dolas_paths.h"

#include "core/dolas_math.h"
#include "rsd/camera.h"
#include "rsd/scene.h"
#include "rsd/entity.h"
#include "rsd/material.h"
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
        TRsd* GetRsdAsset(const std::string& file_name);

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

        static bool IsAbsolutePathLike(const std::string& p)
        {
            if (p.size() >= 2 && ((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')) && p[1] == ':')
                return true; // Windows: C:\...
            if (!p.empty() && (p[0] == '/' || p[0] == '\\'))
                return true;
            return false;
        }

        std::unordered_map<std::type_index, std::unique_ptr<IRsdCache>> m_rsd_caches;
    };

    template<class TRsd>
    TRsd* AssetManager::GetRsdAsset(const std::string& file_name)
    {
		// Accepts an absolute path; otherwise, concatenates with the content's relative path.
        // If an absolute path is provided, use it directly; otherwise, resolve it relative to the content directory.
        const std::string file_path = IsAbsolutePathLike(file_name) ? file_name : (PathUtils::GetContentDir() + file_name);
        std::unordered_map<std::string, TRsd>& cache = GetTypedCache<TRsd>().map;
        auto it = cache.find(file_path);

        if (it != cache.end())
        {
            // Cache hit
			return &it->second;
        }
        else
        {
            // Cache miss
			TRsd value{};
            if (!LoadAndParseRsdFile(file_path, &value, TRsd::kFields.data(), TRsd::kFields.size()))
            {
                return nullptr;
            }

			auto [ins, _] = cache.emplace(file_path, std::move(value));
			return &ins->second;
        }
    }
}
#endif // DOLAS_ASSET_MANAGER_H