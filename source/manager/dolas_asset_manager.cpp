#include "manager/dolas_asset_manager.h"
#include "base/dolas_base.h"
#include <fstream>
#include <iostream>
#include "base/dolas_paths.h"
#include "manager/dolas_log_system_manager.h"
namespace Dolas
{
    AssetManager::AssetManager()
    {
    }

    AssetManager::~AssetManager()
    {
        Clear();
    }

    Bool AssetManager::Initialize()
    {
        return true;
    }

    Bool AssetManager::Clear()
    {
        // 清理相机资产
        for (auto& pair : m_camera_asset_map)
        {
            CameraAsset* camera_asset = pair.second;
            if (camera_asset)
            {
                DOLAS_DELETE(camera_asset);
            }
        }
        m_camera_asset_map.clear();

        // 清理场景资产
        for (auto& pair : m_scene_asset_map)
        {
            SceneAsset* scene_asset = pair.second;
            if (scene_asset)
            {
                DOLAS_DELETE(scene_asset);
            }
        }
        m_scene_asset_map.clear();

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
			LOG_ERROR("Unknown camera perspective type!");
			DOLAS_DELETE(camera_asset);
			return nullptr;
		}

		return camera_asset;
    }

	SceneAsset* AssetManager::parseJsonToSceneAsset(const json& json_data)
	{
		SceneAsset* scene_asset = nullptr;
		if (json_data.contains("entities") && json_data["entities"].is_array())
		{
			scene_asset = DOLAS_NEW(SceneAsset);
			for (const auto& entity_json : json_data["entities"])
			{
				if (entity_json.contains("name") && entity_json["name"].is_string() &&
					entity_json.contains("entity_file") && entity_json["entity_file"].is_string() &&
					entity_json.contains("position") && entity_json["position"].is_array() &&
					entity_json.contains("rotation") && entity_json["rotation"].is_array()&&
					entity_json.contains("scale") && entity_json["scale"].is_array())
				{
					SceneEntity scene_entity;
					scene_entity.name = entity_json["name"];
					scene_entity.entity_file = entity_json["entity_file"];
					scene_entity.position = Vector3(entity_json["position"][0], entity_json["position"][1], entity_json["position"][2]);
					scene_entity.rotation = Quaternion(entity_json["rotation"][0], entity_json["rotation"][1], entity_json["rotation"][2], entity_json["rotation"][3]);
					scene_entity.scale = Vector3(entity_json["scale"][0], entity_json["scale"][1], entity_json["scale"][2]);
					scene_asset->entities.push_back(scene_entity);
				}
				else
				{
					LOG_ERROR("Invalid scene entity format in JSON!");
				}
			}
			for (const auto& object_name : json_data["models"])
			{
				scene_asset->model_names.push_back(object_name["name"]);
			}
		}
		else
		{
			LOG_ERROR("Scene JSON does not contain valid entities array!");
		}
		return scene_asset;
	}
}