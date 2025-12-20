#include "manager/dolas_asset_manager.h"
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "manager/dolas_log_system_manager.h"
#include "tinyxml2.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <map>
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
        // 某些编译环境下 std::unordered_map::clear() 可能被宏/头文件污染导致报错；
        // 这里用“赋空”方式等效清空，避免依赖 clear() 名称查找。
        m_camera_rsd_asset_map = decltype(m_camera_rsd_asset_map){};
        m_scene_rsd_asset_map = decltype(m_scene_rsd_asset_map){};
        m_entity_rsd_asset_map = decltype(m_entity_rsd_asset_map){};
        m_material_rsd_asset_map = decltype(m_material_rsd_asset_map){};

        return true;
    }

    static Bool LoadXmlFileInternal(const std::string& file_path, tinyxml2::XMLDocument& xml_doc)
    {
        const auto ret = xml_doc.LoadFile(file_path.c_str());
        return ret == tinyxml2::XML_SUCCESS;
    }

    static bool ParseFieldInto(void* outBase, const RsdFieldDesc& f, const tinyxml2::XMLElement* root)
    {
        const tinyxml2::XMLElement* el = root ? root->FirstChildElement(f.name) : nullptr;

        auto* base = reinterpret_cast<char*>(outBase);

        switch (f.type)
        {
        case RsdFieldType::String:
        {
            auto* p = reinterpret_cast<std::string*>(base + f.offset);
            if (el && el->GetText()) *p = el->GetText();
            return true;
        }
        case RsdFieldType::BoolValue:
        {
            auto* p = reinterpret_cast<Bool*>(base + f.offset);
            if (el && el->GetText())
            {
                std::string t = el->GetText();
                *p = (t == "true" || t == "1");
            }
            return true;
        }
        case RsdFieldType::IntValue:
        {
            auto* p = reinterpret_cast<Int*>(base + f.offset);
            if (el && el->GetText()) *p = (Int)std::strtol(el->GetText(), nullptr, 10);
            return true;
        }
        case RsdFieldType::UIntValue:
        {
            auto* p = reinterpret_cast<UInt*>(base + f.offset);
            if (el && el->GetText()) *p = (UInt)std::strtoul(el->GetText(), nullptr, 10);
            return true;
        }
        case RsdFieldType::FloatValue:
        {
            auto* p = reinterpret_cast<Float*>(base + f.offset);
            if (el && el->GetText()) *p = (Float)std::atof(el->GetText());
            return true;
        }
        case RsdFieldType::Vector3:
        {
            auto* p = reinterpret_cast<Vector3*>(base + f.offset);
            if (!el) return false;
            double x=0,y=0,z=0;
            if (el->QueryDoubleAttribute("x",&x)!=tinyxml2::XML_SUCCESS) return false;
            if (el->QueryDoubleAttribute("y",&y)!=tinyxml2::XML_SUCCESS) return false;
            if (el->QueryDoubleAttribute("z",&z)!=tinyxml2::XML_SUCCESS) return false;
            *p = Vector3((Float)x,(Float)y,(Float)z);
            return true;
        }
        case RsdFieldType::Vector4:
        {
            auto* p = reinterpret_cast<Vector4*>(base + f.offset);
            if (!el) return false;
            double x=0,y=0,z=0,w=0;
            if (el->QueryDoubleAttribute("x",&x)!=tinyxml2::XML_SUCCESS) return false;
            if (el->QueryDoubleAttribute("y",&y)!=tinyxml2::XML_SUCCESS) return false;
            if (el->QueryDoubleAttribute("z",&z)!=tinyxml2::XML_SUCCESS) return false;
            if (el->QueryDoubleAttribute("w",&w)!=tinyxml2::XML_SUCCESS) return false;
            *p = Vector4((Float)x,(Float)y,(Float)z,(Float)w);
            return true;
        }
        case RsdFieldType::DynArrayXml:
        {
            auto* p = reinterpret_cast<std::vector<std::string>*>(base + f.offset);
            if (!el) return true;
            for (auto* child = el->FirstChildElement(); child; child = child->NextSiblingElement())
            {
                tinyxml2::XMLPrinter pr;
                child->Accept(&pr);
                p->emplace_back(pr.CStr());
            }
            return true;
        }
        case RsdFieldType::MapStringVector4:
        {
            auto* p = reinterpret_cast<std::map<std::string, Vector4>*>(base + f.offset);
            if (!el) return true;
            for (auto* v = el->FirstChildElement("vec4"); v; v = v->NextSiblingElement("vec4"))
            {
                const char* n = v->Attribute("name");
                if (!n) continue;
                double x=0,y=0,z=0,w=0;
                if (v->QueryDoubleAttribute("x",&x)!=tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("y",&y)!=tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("z",&z)!=tinyxml2::XML_SUCCESS) continue;
                if (v->QueryDoubleAttribute("w",&w)!=tinyxml2::XML_SUCCESS) continue;
                (*p)[std::string(n)] = Vector4((Float)x,(Float)y,(Float)z,(Float)w);
            }
            return true;
        }
        case RsdFieldType::MapStringString:
        {
            auto* p = reinterpret_cast<std::map<std::string, std::string>*>(base + f.offset);
            if (!el) return true;
            for (auto* t = el->FirstChildElement("texture"); t; t = t->NextSiblingElement("texture"))
            {
                const char* n = t->Attribute("name");
                const char* f2 = t->Attribute("file");
                if (n && f2) (*p)[std::string(n)] = std::string(f2);
            }
            return true;
        }
        case RsdFieldType::MapStringFloat:
        {
            auto* p = reinterpret_cast<std::map<std::string, Float>*>(base + f.offset);
            if (!el) return true;
            for (auto* t = el->FirstChildElement("float"); t; t = t->NextSiblingElement("float"))
            {
                const char* n = t->Attribute("name");
                const char* v = t->Attribute("value");
                if (n && v) (*p)[std::string(n)] = (Float)std::atof(v);
            }
            return true;
        }
        case RsdFieldType::Unsupported:
        default:
            return true;
        }
    }

    template <typename TRsd>
    static bool ParseRsdFromXml(const tinyxml2::XMLElement* root, TRsd& out)
    {
        bool ok = true;
        for (std::size_t i = 0; i < TRsd::kFieldCount; i++)
        {
            const auto& f = TRsd::kFields[i];
            if (!ParseFieldInto(&out, f, root))
                ok = false;
        }
        return ok;
    }

    template <typename TRsd>
    static TRsd* GetRsdFromDir(
        const std::string& dir,
        const std::string& file_name,
        std::unordered_map<std::string, TRsd>& cache)
    {
        const std::string file_path = dir + file_name;

        auto it = cache.find(file_path);
        if (it != cache.end())
            return &it->second;

        tinyxml2::XMLDocument doc;
        if (!LoadXmlFileInternal(file_path, doc))
        {
            LOG_ERROR("Failed to load xml: {}", file_path);
            return nullptr;
        }

        TRsd value{};
        if (!ParseRsdFromXml<TRsd>(doc.RootElement(), value))
        {
            LOG_ERROR("Failed to parse RSD from xml: {}", file_path);
            return nullptr;
        }

        auto [ins, _] = cache.emplace(file_path, std::move(value));
        return &ins->second;
    }

	SceneRSD* AssetManager::GetSceneAsset(const std::string& file_name)
	{
        return GetRsdFromDir<SceneRSD>(PathUtils::GetSceneDir(), file_name, m_scene_rsd_asset_map);
	}

    CameraRSD* AssetManager::GetCameraRSDAsset(const std::string& file_name)
    {
        return GetRsdFromDir<CameraRSD>(PathUtils::GetCameraDir(), file_name, m_camera_rsd_asset_map);
    }

    EntityRSD* AssetManager::GetEntityRSDAsset(const std::string& file_name)
    {
        return GetRsdFromDir<EntityRSD>(PathUtils::GetEntityDir(), file_name, m_entity_rsd_asset_map);
    }

    MaterialRSD* AssetManager::GetMaterialRSDAsset(const std::string& file_name)
    {
        return GetRsdFromDir<MaterialRSD>(PathUtils::GetMaterialDir(), file_name, m_material_rsd_asset_map);
    }


}