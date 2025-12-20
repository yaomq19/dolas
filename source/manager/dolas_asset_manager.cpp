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
        for (auto& kv : m_rsd_caches)
        {
            if (kv.second) kv.second->Clear();
        }
        m_rsd_caches.clear();

        return true;
    }

    static Bool LoadXmlFileInternal(const std::string& file_path, tinyxml2::XMLDocument& xml_doc)
    {
        const auto ret = xml_doc.LoadFile(file_path.c_str());
        return ret == tinyxml2::XML_SUCCESS;
    }

    /**
     * Parse a single field described by RSD reflection metadata from an XML document into a C++ struct.
     *
     * This function is the core of the "generic RSD loader" path. Precompile generates, for each RSD
     * struct (e.g. CameraRSD), a static table `kFields[]` where each entry describes:
     * - the XML field name (`RsdFieldDesc::name`)
     * - the logical field type (`RsdFieldDesc::type`, an enum RsdFieldType)
     * - the byte offset of the field within the struct (`RsdFieldDesc::offset`, computed with offsetof)
     *
     * Given a base pointer to an output struct instance (outBase), we compute:
     *     char* base = reinterpret_cast<char*>(outBase);
     * and then write to the field location via `base + offset`.
     *
     * Safety / assumptions:
     * - `outBase` must point to a fully constructed TRsd instance (not nullptr).
     * - `f.offset` must match the memory layout of that TRsd type.
     * - The field type in `f.type` must match the actual C++ member type at that offset
     *   (e.g. String -> std::string, Vector3 -> Dolas::Vector3, MapStringVector4 -> std::map<std::string, Vector4>, etc.).
     *   If these drift, behavior is undefined (reinterpret_cast into the wrong type).
     *
     * Parsing rules:
     * - For scalar text nodes (String/Bool/Int/UInt/Float): we read the element text and convert.
     * - For Vector3/Vector4: we read numeric attributes x/y/z(/w) on the element.
     * - For DynamicArray<Xml> (DynArrayXml): we serialize each child element back into an XML fragment string.
     * - For Map<String, Vector4>: expects `<vec4 name="..." x=".." y=".." z=".." w=".."/>` children.
     * - For Map<String, String>: expects `<texture name="..." file="..."/>` children.
     * - For Map<String, Float>: expects `<float name="..." value="..."/>` children.
     *
     * Return value:
     * - Returns true if parsing succeeded for this field.
     * - Returns false when a field is present but malformed in a way we treat as a hard failure
     *   (currently mainly vector attribute parsing).
     * - For unsupported/unknown field types we return true (no-op) to keep the loader forward-compatible.
     *
     * Note: Missing elements for many field kinds are treated as "keep default value" and return true.
     */
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
        for (std::size_t i = 0; i < TRsd::kFields.size(); i++)
        {
            const auto& f = TRsd::kFields[i];
            if (!ParseFieldInto(&out, f, root))
                ok = false;
        }
        return ok;
	}

    bool AssetManager::LoadAndParseRsdFile(const std::string& file_path, void* outBase, const RsdFieldDesc* fields, std::size_t fieldCount)
    {
        tinyxml2::XMLDocument doc;
        if (!LoadXmlFileInternal(file_path, doc))
        {
            LOG_ERROR("Failed to load xml: {}", file_path);
            return false;
        }

        const tinyxml2::XMLElement* root = doc.RootElement();
        if (!root)
        {
            LOG_ERROR("Invalid xml root: {}", file_path);
            return false;
        }

        bool ok = true;
        for (std::size_t i = 0; i < fieldCount; i++)
        {
            if (!ParseFieldInto(outBase, fields[i], root))
            {
                ok = false;
            }
        }
        if (!ok)
        {
            LOG_ERROR("Failed to parse RSD from xml: {}", file_path);
        }
        return ok;
    }


}