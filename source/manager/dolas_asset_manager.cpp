#include "manager/dolas_asset_manager.h"
#include "base/dolas_base.h"
#include <fstream>
#include <iostream>
#include "base/dolas_paths.h"
namespace Dolas
{
    AssetManager::AssetManager()
    {
    }

    AssetManager::~AssetManager()
    {
    }

    Bool AssetManager::Initialize()
    {
        return true;
    }

    Bool AssetManager::Clear()
    {
        return true;
    }

    Bool AssetManager::LoadJsonFile(const std::string& file_path, json& json_data)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            return false;
        }
        file >> json_data;
        file.close();
        return true;
    }

    json AssetManager::LoadJsonFile(const std::string& file_path)
	{
		json json_data;
		std::ifstream file(file_path);
		if (!file.is_open())
		{
			return false;
		}
		file >> json_data;
		file.close();
		return json_data;
	}

    CameraAsset* AssetManager::GetCameraAsset(const std::string& file_name)
    {
        std::string camera_dir_path = PathUtils::GetCameraDir();
		std::string file_path = camera_dir_path + file_name;

        auto iter = m_camera_asset_map.find(file_path);
		if (iter != m_camera_asset_map.end() && iter->second != nullptr)
		{
			return m_camera_asset_map[file_path];
		}

        json json_data = LoadJsonFile(file_path);

		CameraAsset* camera_asset = parseJsonToCameraAsset(json_data);
		m_camera_asset_map[file_path] = camera_asset;
        return m_camera_asset_map[file_path];
    }

	SceneAsset* AssetManager::GetSceneAsset(const std::string& file_name)
	{
		std::string scene_dir_path = PathUtils::GetSceneDir();
		std::string file_path = scene_dir_path + file_name;

		auto iter = m_scene_asset_map.find(file_path);
		if (iter != m_scene_asset_map.end() && iter->second != nullptr)
		{
			return m_scene_asset_map[file_path];
		}

		json json_data = LoadJsonFile(file_path);

		SceneAsset* scene_asset = parseJsonToSceneAsset(json_data);
		m_scene_asset_map[file_path] = scene_asset;
		return m_scene_asset_map[file_path];
	}

    CameraAsset* AssetManager::parseJsonToCameraAsset(const json& json_data)
    {
		CameraAsset* camera_asset = DOLAS_NEW(CameraAsset);

        if (json_data["camera_perspective_type"] == "perspective")
        {
			camera_asset->perspective_type = "perspective";
			camera_asset->position = Vector3(json_data["position"][0], json_data["position"][1], json_data["position"][2]);
			camera_asset->forward = Vector3(json_data["forward"][0], json_data["forward"][1], json_data["forward"][2]);
			camera_asset->up = Vector3(json_data["up"][0], json_data["up"][1], json_data["up"][2]);
			camera_asset->near_plane = json_data["near_plane"];
			camera_asset->far_plane = json_data["far_plane"];
			camera_asset->fov = json_data["fov"];
			camera_asset->aspect_ratio = json_data["aspect_ratio"];
		}
		else if (json_data["camera_perspective_type"] == "orthographic")
		{
			camera_asset->perspective_type = "orthographic";
			camera_asset->position = Vector3(json_data["position"][0], json_data["position"][1], json_data["position"][2]);
			camera_asset->forward = Vector3(json_data["forward"][0], json_data["forward"][1], json_data["forward"][2]);
			camera_asset->up = Vector3(json_data["up"][0], json_data["up"][1], json_data["up"][2]);
			camera_asset->near_plane = json_data["near_plane"];
			camera_asset->far_plane = json_data["far_plane"];
			camera_asset->window_width = json_data["window_width"];
			camera_asset->window_height = json_data["window_height"];
		}
		else
		{
			std::cout << "Unknown camera perspective type!" << std::endl;
			DOLAS_DELETE(camera_asset);
			return nullptr;
		}

		return camera_asset;
    }

	SceneAsset* AssetManager::parseJsonToSceneAsset(const json& json_data)
	{

		return nullptr;
	}
}