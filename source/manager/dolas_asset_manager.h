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

    struct CameraAsset
    {
        std::string perspective_type;
        Vector3 position;
        Vector3 forward;
        Vector3 up;
        Float near_plane;
        Float far_plane;
        // only for perspective
        Float fov;
        Float aspect_ratio; // may be overwrite by client real width / height
        // only for ortho
        Float window_width;
        Float window_height;
    };

	struct SceneEntity
	{
		std::string name;
		std::string entity_file;
		Vector3 position;
        Quaternion rotation;
		Vector3 scale;
	};

    struct SceneAsset
    {
		std::vector<SceneEntity> entities;
        std::vector<std::string> model_names;
    };

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
        // 允许传入绝对路径；否则按 content 相对路径拼接
        const std::string file_path = IsAbsolutePathLike(file_name) ? file_name : (PathUtils::GetContentDir() + file_name);

        auto& cache = GetTypedCache<TRsd>().map;
        auto it = cache.find(file_path);
        if (it != cache.end())
            return &it->second;

        TRsd value{};
        if (!LoadAndParseRsdFile(file_path, &value, TRsd::kFields, TRsd::kFieldCount))
            return nullptr;

        auto [ins, _] = cache.emplace(file_path, std::move(value));
        return &ins->second;
    }
}
#endif // DOLAS_ASSET_MANAGER_H