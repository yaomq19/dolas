#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "base/dolas_base.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "core/dolas_math.h"
#include "rsd/camera.h"
#include "rsd/scene.h"
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

        Bool LoadJsonFile(const std::string& file_path, json& json_data);
        
        SceneRSD* GetSceneAsset(const std::string& file_name);
        CameraRSD* GetCameraRSDAsset(const std::string& file_name);
    protected:
        json LoadJsonFile(const std::string& file_path);
        CameraAsset* parseJsonToCameraAsset(const json& json_data);
		SceneAsset* parseJsonToSceneAsset(const json& json_data);
        CameraRSD* parseJsonToCameraRSDAsset(const json& json_data);
        SceneRSD* parseJsonToSceneRSDAsset(const json& json_data);

		std::unordered_map<std::string, CameraAsset*> m_camera_asset_map;
		std::unordered_map<std::string, SceneAsset*> m_scene_asset_map;
        std::unordered_map<std::string, CameraRSD> m_camera_rsd_asset_map;
        std::unordered_map<std::string, SceneRSD> m_scene_rsd_asset_map;
    };
}
#endif // DOLAS_ASSET_MANAGER_H