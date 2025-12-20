#include "manager/dolas_asset_manager.h"
#include "base/dolas_base.h"
#include <fstream>
#include <iostream>
#include "base/dolas_paths.h"
#include "manager/dolas_log_system_manager.h"
#include "tinyxml2.h"
#include <cstdlib>
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

        // 清理 RSD 相机资产（值类型）
        m_camera_rsd_asset_map.clear();

        // 清理 RSD 场景资产（值类型）
        m_scene_rsd_asset_map.clear();

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

    Bool AssetManager::LoadXmlFile(const std::string& file_path, tinyxml2::XMLDocument& xml_doc)
    {
        const auto ret = xml_doc.LoadFile(file_path.c_str());
        return ret == tinyxml2::XML_SUCCESS;
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

	SceneRSD* AssetManager::GetSceneAsset(const std::string& file_name)
	{
		std::string scene_dir_path = PathUtils::GetSceneDir();
		std::string file_path = scene_dir_path + file_name;

		auto iter = m_scene_rsd_asset_map.find(file_path);
		if (iter != m_scene_rsd_asset_map.end())
		{
			return &iter->second;
		}

        tinyxml2::XMLDocument doc;
        if (!LoadXmlFile(file_path, doc))
        {
            LOG_ERROR("Failed to load scene xml: {}", file_path);
            return nullptr;
        }

        SceneRSD* scene_rsd = parseXmlToSceneRSDAsset(doc);
        if (scene_rsd == nullptr)
        {
            LOG_ERROR("Failed to parse scene RSD from json: {}", file_path);
            return nullptr;
        }

        auto [it, _] = m_scene_rsd_asset_map.emplace(file_path, *scene_rsd);
        DOLAS_DELETE(scene_rsd);
        return &it->second;
	}

    CameraRSD* AssetManager::GetCameraRSDAsset(const std::string& file_name)
    {
        std::string camera_dir_path = PathUtils::GetCameraDir();
        std::string file_path = camera_dir_path + file_name;

        auto iter = m_camera_rsd_asset_map.find(file_path);
        if (iter != m_camera_rsd_asset_map.end())
        {
            return &iter->second;
        }

        tinyxml2::XMLDocument doc;
        if (!LoadXmlFile(file_path, doc))
        {
            LOG_ERROR("Failed to load camera xml: {}", file_path);
            return nullptr;
        }

        CameraRSD* camera_rsd = parseXmlToCameraRSDAsset(doc);
        if (camera_rsd == nullptr)
        {
            LOG_ERROR("Failed to parse camera RSD from json: {}", file_path);
            return nullptr;
        }

        auto [it, _] = m_camera_rsd_asset_map.emplace(file_path, *camera_rsd);
        DOLAS_DELETE(camera_rsd);
        return &it->second;
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

    CameraRSD* AssetManager::parseJsonToCameraRSDAsset(const json& json_data)
    {
        CameraRSD* camera_asset = DOLAS_NEW(CameraRSD);

        auto read_vec3 = [&](const char* key, Vector3& out) -> Bool
        {
            if (!json_data.contains(key) || !json_data[key].is_array() || json_data[key].size() < 3)
                return false;
            out = Vector3(json_data[key][0], json_data[key][1], json_data[key][2]);
            return true;
        };

        camera_asset->camera_perspective_type = json_data.value("camera_perspective_type", "");

        if (!read_vec3("position", camera_asset->position) ||
            !read_vec3("forward", camera_asset->forward) ||
            !read_vec3("up", camera_asset->up))
        {
            LOG_ERROR("Invalid camera vectors in JSON!");
            DOLAS_DELETE(camera_asset);
            return nullptr;
        }

        // 统一按字段存在与否读取；缺失时保持默认值（0）
        if (json_data.contains("near_plane")) camera_asset->near_plane = json_data["near_plane"];
        if (json_data.contains("far_plane")) camera_asset->far_plane = json_data["far_plane"];
        if (json_data.contains("fov")) camera_asset->fov = json_data["fov"];
        if (json_data.contains("aspect_ratio")) camera_asset->aspect_ratio = json_data["aspect_ratio"];
        if (json_data.contains("window_width")) camera_asset->window_width = json_data["window_width"];
        if (json_data.contains("window_height")) camera_asset->window_height = json_data["window_height"];

        // 基本校验：perspective/orthographic 之外也允许，但记录一下
        if (camera_asset->camera_perspective_type != "perspective" &&
            camera_asset->camera_perspective_type != "orthographic")
        {
            LOG_ERROR("Unknown camera perspective type: {}", camera_asset->camera_perspective_type);
        }

        return camera_asset;
    }

    SceneRSD* AssetManager::parseJsonToSceneRSDAsset(const json& json_data)
    {
        SceneRSD* scene_rsd = DOLAS_NEW(SceneRSD);

        // entities
        if (json_data.contains("entities") && json_data["entities"].is_array())
        {
            for (const auto& e : json_data["entities"])
            {
                scene_rsd->entities.push_back(e);
            }
        }
        else
        {
            // 允许为空，但给个提示
            LOG_WARN("Scene JSON does not contain entities array (or it's not an array).");
        }

        // models
        if (json_data.contains("models") && json_data["models"].is_array())
        {
            for (const auto& m : json_data["models"])
            {
                scene_rsd->models.push_back(m);
            }
        }

        return scene_rsd;
    }

    // ===== XML 版本（RSD 系统使用）=====
    static const tinyxml2::XMLElement* FindChild(const tinyxml2::XMLElement* root, const char* name)
    {
        return root ? root->FirstChildElement(name) : nullptr;
    }

    static std::string ChildText(const tinyxml2::XMLElement* root, const char* name)
    {
        auto* el = root ? root->FirstChildElement(name) : nullptr;
        const char* t = el ? el->GetText() : nullptr;
        return t ? std::string(t) : std::string();
    }

    static bool ReadVector3(const tinyxml2::XMLElement* root, const char* name, Vector3& out)
    {
        auto* el = FindChild(root, name);
        if (!el) return false;
        double x = 0, y = 0, z = 0;
        if (el->QueryDoubleAttribute("x", &x) != tinyxml2::XML_SUCCESS) return false;
        if (el->QueryDoubleAttribute("y", &y) != tinyxml2::XML_SUCCESS) return false;
        if (el->QueryDoubleAttribute("z", &z) != tinyxml2::XML_SUCCESS) return false;
        out = Vector3(static_cast<Float>(x), static_cast<Float>(y), static_cast<Float>(z));
        return true;
    }

    static bool ReadVector4WXYZ(const tinyxml2::XMLElement* root, const char* name, Quaternion& out)
    {
        auto* el = FindChild(root, name);
        if (!el) return false;
        double w = 1, x = 0, y = 0, z = 0;
        if (el->QueryDoubleAttribute("w", &w) != tinyxml2::XML_SUCCESS) return false;
        if (el->QueryDoubleAttribute("x", &x) != tinyxml2::XML_SUCCESS) return false;
        if (el->QueryDoubleAttribute("y", &y) != tinyxml2::XML_SUCCESS) return false;
        if (el->QueryDoubleAttribute("z", &z) != tinyxml2::XML_SUCCESS) return false;
        out = Quaternion(static_cast<Float>(w), static_cast<Float>(x), static_cast<Float>(y), static_cast<Float>(z));
        return true;
    }

    static bool ReadFloatChild(const tinyxml2::XMLElement* root, const char* name, Float& out)
    {
        auto* el = FindChild(root, name);
        if (!el || !el->GetText()) return false;
        out = static_cast<Float>(std::atof(el->GetText()));
        return true;
    }

    CameraRSD* AssetManager::parseXmlToCameraRSDAsset(const tinyxml2::XMLDocument& doc)
    {
        const tinyxml2::XMLElement* root = doc.RootElement();
        if (!root) return nullptr;

        CameraRSD* camera = DOLAS_NEW(CameraRSD);
        camera->camera_perspective_type = ChildText(root, "camera_perspective_type");

        if (!ReadVector3(root, "position", camera->position) ||
            !ReadVector3(root, "forward", camera->forward) ||
            !ReadVector3(root, "up", camera->up))
        {
            LOG_ERROR("Invalid camera vectors in XML!");
            DOLAS_DELETE(camera);
            return nullptr;
        }

        // 标量（缺失则保持默认 0）
        ReadFloatChild(root, "near_plane", camera->near_plane);
        ReadFloatChild(root, "far_plane", camera->far_plane);
        ReadFloatChild(root, "fov", camera->fov);
        ReadFloatChild(root, "aspect_ratio", camera->aspect_ratio);
        ReadFloatChild(root, "window_width", camera->window_width);
        ReadFloatChild(root, "window_height", camera->window_height);

        return camera;
    }

    SceneRSD* AssetManager::parseXmlToSceneRSDAsset(const tinyxml2::XMLDocument& doc)
    {
        const tinyxml2::XMLElement* root = doc.RootElement();
        if (!root) return nullptr;

        SceneRSD* scene = DOLAS_NEW(SceneRSD);
        const tinyxml2::XMLElement* entities = root->FirstChildElement("entities");
        if (entities)
        {
            for (auto* e = entities->FirstChildElement("entity"); e; e = e->NextSiblingElement("entity"))
            {
                tinyxml2::XMLPrinter p;
                e->Accept(&p);
                scene->entities.emplace_back(p.CStr());
            }
        }

        const tinyxml2::XMLElement* models = root->FirstChildElement("models");
        if (models)
        {
            for (auto* m = models->FirstChildElement("model"); m; m = m->NextSiblingElement("model"))
            {
                tinyxml2::XMLPrinter p;
                m->Accept(&p);
                scene->models.emplace_back(p.CStr());
            }
        }

        return scene;
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