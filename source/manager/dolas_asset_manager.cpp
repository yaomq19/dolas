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
        m_camera_rsd_asset_map.clear();
        m_scene_rsd_asset_map.clear();
        m_entity_rsd_asset_map.clear();
        m_material_rsd_asset_map.clear();

        return true;
    }

    Bool AssetManager::LoadXmlFile(const std::string& file_path, tinyxml2::XMLDocument& xml_doc)
    {
        const auto ret = xml_doc.LoadFile(file_path.c_str());
        return ret == tinyxml2::XML_SUCCESS;
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

    EntityRSD* AssetManager::GetEntityRSDAsset(const std::string& file_name)
    {
        std::string dir = PathUtils::GetEntityDir();
        std::string file_path = dir + file_name;

        auto iter = m_entity_rsd_asset_map.find(file_path);
        if (iter != m_entity_rsd_asset_map.end())
            return &iter->second;

        tinyxml2::XMLDocument doc;
        if (!LoadXmlFile(file_path, doc))
        {
            LOG_ERROR("Failed to load entity xml: {}", file_path);
            return nullptr;
        }

        EntityRSD* e = parseXmlToEntityRSDAsset(doc);
        if (!e)
        {
            LOG_ERROR("Failed to parse entity RSD from xml: {}", file_path);
            return nullptr;
        }

        auto [it, _] = m_entity_rsd_asset_map.emplace(file_path, *e);
        DOLAS_DELETE(e);
        return &it->second;
    }

    MaterialRSD* AssetManager::GetMaterialRSDAsset(const std::string& file_name)
    {
        std::string dir = PathUtils::GetMaterialDir();
        std::string file_path = dir + file_name;

        auto iter = m_material_rsd_asset_map.find(file_path);
        if (iter != m_material_rsd_asset_map.end())
            return &iter->second;

        tinyxml2::XMLDocument doc;
        if (!LoadXmlFile(file_path, doc))
        {
            LOG_ERROR("Failed to load material xml: {}", file_path);
            return nullptr;
        }

        MaterialRSD* m = parseXmlToMaterialRSDAsset(doc);
        if (!m)
        {
            LOG_ERROR("Failed to parse material RSD from xml: {}", file_path);
            return nullptr;
        }

        auto [it, _] = m_material_rsd_asset_map.emplace(file_path, *m);
        DOLAS_DELETE(m);
        return &it->second;
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

    EntityRSD* AssetManager::parseXmlToEntityRSDAsset(const tinyxml2::XMLDocument& doc)
    {
        const tinyxml2::XMLElement* root = doc.RootElement();
        if (!root) return nullptr;

        auto child_text = [&](const char* name) -> std::string
        {
            const tinyxml2::XMLElement* el = root->FirstChildElement(name);
            const char* t = el ? el->GetText() : nullptr;
            return t ? std::string(t) : std::string();
        };

        EntityRSD* e = DOLAS_NEW(EntityRSD);
        e->type = child_text("type");
        e->mesh = child_text("mesh");
        e->base_geometry = child_text("base_geometry");
        e->material = child_text("material");
        return e;
    }

    MaterialRSD* AssetManager::parseXmlToMaterialRSDAsset(const tinyxml2::XMLDocument& doc)
    {
        const tinyxml2::XMLElement* root = doc.RootElement();
        if (!root) return nullptr;

        auto child_text = [&](const char* name) -> std::string
        {
            const tinyxml2::XMLElement* el = root->FirstChildElement(name);
            const char* t = el ? el->GetText() : nullptr;
            return t ? std::string(t) : std::string();
        };

        MaterialRSD* m = DOLAS_NEW(MaterialRSD);
        m->vertex_shader = child_text("vertex_shader");
        m->pixel_shader = child_text("pixel_shader");

        auto parse_vec4_block = [&](const char* blockName, std::map<std::string, Vector4>& out)
        {
            const tinyxml2::XMLElement* block = root->FirstChildElement(blockName);
            if (!block) return;
            for (auto* v = block->FirstChildElement("vec4"); v; v = v->NextSiblingElement("vec4"))
            {
                const char* n = v->Attribute("name");
                if (!n) continue;
                double x = 0, y = 0, z = 0, w = 0;
                if (v->QueryDoubleAttribute("x", &x) != tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("y", &y) != tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("z", &z) != tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("w", &w) != tinyxml2::XML_SUCCESS) continue;
                out[std::string(n)] = Vector4((Float)x, (Float)y, (Float)z, (Float)w);
            }
        };

        parse_vec4_block("vertex_shader_global_variables", m->vertex_shader_global_variables);
        parse_vec4_block("pixel_shader_global_variables", m->pixel_shader_global_variables);

        // textures: <pixel_shader_texture><texture name="" file=""/></pixel_shader_texture>
        if (auto* texBlock = root->FirstChildElement("pixel_shader_texture"))
        {
            for (auto* t = texBlock->FirstChildElement("texture"); t; t = t->NextSiblingElement("texture"))
            {
                const char* name = t->Attribute("name");
                const char* file = t->Attribute("file");
                if (name && file)
                    m->pixel_shader_texture[std::string(name)] = std::string(file);
            }
        }

        // parameter: <parameter><float name="" value=""/></parameter>
        if (auto* param = root->FirstChildElement("parameter"))
        {
            for (auto* f = param->FirstChildElement("float"); f; f = f->NextSiblingElement("float"))
            {
                const char* name = f->Attribute("name");
                const char* value = f->Attribute("value");
                if (!name || !value) continue;
                m->parameter[std::string(name)] = (Float)std::atof(value);
            }
        }

        return m;
    }

}