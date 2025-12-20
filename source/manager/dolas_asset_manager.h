#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "base/dolas_base.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;
#include "tinyxml2.h"

#include "core/dolas_math.h"
#include "rsd/camera.h"
#include "rsd/scene.h"
#include "rsd/entity.h"
#include "rsd/material.h"
namespace Dolas
{
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

        SceneRSD* GetSceneAsset(const std::string& file_name);
        CameraRSD* GetCameraRSDAsset(const std::string& file_name);
        EntityRSD* GetEntityRSDAsset(const std::string& file_name);
        MaterialRSD* GetMaterialRSDAsset(const std::string& file_name);
    protected:
        // 不对外暴露 XML：仅内部使用
        Bool LoadXmlFile(const std::string& file_path, tinyxml2::XMLDocument& xml_doc);

        // XML 版本（RSD 系统）
        CameraRSD* parseXmlToCameraRSDAsset(const tinyxml2::XMLDocument& doc);
        SceneRSD* parseXmlToSceneRSDAsset(const tinyxml2::XMLDocument& doc);
        EntityRSD* parseXmlToEntityRSDAsset(const tinyxml2::XMLDocument& doc);
        MaterialRSD* parseXmlToMaterialRSDAsset(const tinyxml2::XMLDocument& doc);

        std::unordered_map<std::string, CameraRSD> m_camera_rsd_asset_map;
        std::unordered_map<std::string, SceneRSD> m_scene_rsd_asset_map;
        std::unordered_map<std::string, EntityRSD> m_entity_rsd_asset_map;
        std::unordered_map<std::string, MaterialRSD> m_material_rsd_asset_map;
    };
}
#endif // DOLAS_ASSET_MANAGER_H