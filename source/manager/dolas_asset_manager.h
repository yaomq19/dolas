#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <string>
#include "base/dolas_base.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "core/dolas_math.h"
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

    class AssetManager
    {
    public:
        AssetManager();
        ~AssetManager();

        Bool Initialize();
        Bool Clear();

        Bool LoadJsonFile(const std::string& file_path, json& json_data);
        json LoadJsonFile(const std::string& file_path);

		CameraAsset* GetCameraAsset(const std::string& file_name);

    protected:
        CameraAsset* parseJsonToCameraAsset(const json& json_data);
		std::unordered_map<std::string, CameraAsset*> m_camera_asset_map;
    };
}
#endif // DOLAS_ASSET_MANAGER_H