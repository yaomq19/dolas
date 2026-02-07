#include "dolas_asset_manager.h"
#include "dolas_base.h"
#include "dolas_log_system_manager.h"
#include "tinyxml2.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <map>
#include <cstring>

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

    static const char* GetScalarTextCompat(const tinyxml2::XMLElement* el)
    {
        if (!el) return nullptr;
        return el->GetText();
    }

    static bool ReadDoubleCompat(const tinyxml2::XMLElement* el, const char* attrOrChildName, double& out)
    {
        if (!el || !attrOrChildName) return false;
        if (el->QueryDoubleAttribute(attrOrChildName, &out) == tinyxml2::XML_SUCCESS)
            return true;
        if (auto* c = el->FirstChildElement(attrOrChildName); c && c->GetText())
        {
            out = std::atof(c->GetText());
            return true;
        }
        return false;
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
     * - For DynamicArray<RawReference> (DynArrayRawReference): we serialize each child element back into an XML fragment string.
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
        case RsdFieldType::Object:
        {
            if (!el) return true;
            if (!f.objectFields || f.objectFieldCount == 0) return true;
            // Parse the nested object fields from the element itself.
            return ParseRsdObjectFromXmlElement(el, base + f.offset, f.objectFields, f.objectFieldCount);
        }
        case RsdFieldType::String:
        case RsdFieldType::RawReference:
        case RsdFieldType::AssetReference:
        {
            auto* p = reinterpret_cast<std::string*>(base + f.offset);
            if (const char* t = GetScalarTextCompat(el))
                *p = t;
            return true;
        }
        case RsdFieldType::BoolValue:
        {
            auto* p = reinterpret_cast<Bool*>(base + f.offset);
            if (const char* t0 = GetScalarTextCompat(el))
            {
                std::string t = t0;
                *p = (t == "true" || t == "1");
            }
            return true;
        }
        case RsdFieldType::IntValue:
        {
            auto* p = reinterpret_cast<Int*>(base + f.offset);
            if (const char* t = GetScalarTextCompat(el)) *p = (Int)std::strtol(t, nullptr, 10);
            return true;
        }
        case RsdFieldType::UIntValue:
        {
            auto* p = reinterpret_cast<UInt*>(base + f.offset);
            if (const char* t = GetScalarTextCompat(el)) *p = (UInt)std::strtoul(t, nullptr, 10);
            return true;
        }
        case RsdFieldType::EnumUInt:
        {
            const char* raw = GetScalarTextCompat(el);
            if (!raw)
                return true;

            std::string t = raw;
            // trim
            auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
            while (!t.empty() && isSpace((unsigned char)t.front())) t.erase(t.begin());
            while (!t.empty() && isSpace((unsigned char)t.back())) t.pop_back();

            UInt u = 0;
            bool matched = false;

            // match against enum name/display/alias (case-insensitive for ASCII)
            if (f.enumItems && f.enumItemCount > 0)
            {
                auto eqNoCase = [](const std::string& a, const char* b) -> bool {
                    if (!b) return false;
                    const std::string bb(b);
                    if (a.size() != bb.size()) return false;
                    for (size_t i = 0; i < a.size(); i++)
                    {
                        const unsigned char ca = (unsigned char)a[i];
                        const unsigned char cb = (unsigned char)bb[i];
                        const unsigned char la = (unsigned char)std::tolower(ca);
                        const unsigned char lb = (unsigned char)std::tolower(cb);
                        if (la != lb) return false;
                    }
                    return true;
                };

                for (std::size_t i = 0; i < f.enumItemCount; i++)
                {
                    const auto& it = f.enumItems[i];
                    if (eqNoCase(t, it.name) || eqNoCase(t, it.display) || eqNoCase(t, it.alias))
                    {
                        u = (UInt)it.value;
                        matched = true;
                        break;
                    }
                }
            }

            if (!matched)
            {
                // numeric fallback
                u = (UInt)std::strtoul(t.c_str(), nullptr, 10);
            }

            // The field is an enum class with underlying UInt. Avoid strict-aliasing UB by memcpy.
            std::memcpy(base + f.offset, &u, sizeof(UInt));
            return true;
        }
        case RsdFieldType::FloatValue:
        {
            auto* p = reinterpret_cast<Float*>(base + f.offset);
            if (const char* t = GetScalarTextCompat(el)) *p = (Float)std::atof(t);
            return true;
        }
        case RsdFieldType::Vector3:
        {
            auto* p = reinterpret_cast<Vector3*>(base + f.offset);
            if (!el) return false;
            double x=0,y=0,z=0;
            if (!ReadDoubleCompat(el, "x", x)) return false;
            if (!ReadDoubleCompat(el, "y", y)) return false;
            if (!ReadDoubleCompat(el, "z", z)) return false;
            *p = Vector3((Float)x,(Float)y,(Float)z);
            return true;
        }
        case RsdFieldType::Vector4:
        {
            auto* p = reinterpret_cast<Vector4*>(base + f.offset);
            if (!el) return false;
            double x=0,y=0,z=0,w=0;
            if (!ReadDoubleCompat(el, "x", x)) return false;
            if (!ReadDoubleCompat(el, "y", y)) return false;
            if (!ReadDoubleCompat(el, "z", z)) return false;
            if (!ReadDoubleCompat(el, "w", w)) return false;
            *p = Vector4((Float)x,(Float)y,(Float)z,(Float)w);
            return true;
        }
        case RsdFieldType::DynArrayRawReference:
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
        case RsdFieldType::DynArrayObject:
        {
            // Requires a typed parse callback generated by precompile (member is std::vector<T>).
            if (!el) return true;
            if (!f.dynArrayObjectParse) return true;
            return f.dynArrayObjectParse(base + f.offset, el);
            return true;
        }
        case RsdFieldType::DynArrayString:
        {
            auto* p = reinterpret_cast<std::vector<std::string>*>(base + f.offset);
            if (!el) return true;
            for (auto* child = el->FirstChildElement(); child; child = child->NextSiblingElement())
            {
                if (const char* v = GetScalarTextCompat(child))
                    p->emplace_back(v);
            }
            return true;
        }
        case RsdFieldType::DynArrayVector3:
        {
            auto* p = reinterpret_cast<std::vector<Vector3>*>(base + f.offset);
            if (!el) return true;
            for (auto* child = el->FirstChildElement(); child; child = child->NextSiblingElement())
            {
                double x=0,y=0,z=0;
                if (!ReadDoubleCompat(child, "x", x)) continue;
                if (!ReadDoubleCompat(child, "y", y)) continue;
                if (!ReadDoubleCompat(child, "z", z)) continue;
                p->emplace_back(Vector3((Float)x,(Float)y,(Float)z));
            }
            return true;
        }
        case RsdFieldType::DynArrayVector4:
        {
            auto* p = reinterpret_cast<std::vector<Vector4>*>(base + f.offset);
            if (!el) return true;
            for (auto* child = el->FirstChildElement(); child; child = child->NextSiblingElement())
            {
                double x=0,y=0,z=0,w=0;
                if (!ReadDoubleCompat(child, "x", x)) continue;
                if (!ReadDoubleCompat(child, "y", y)) continue;
                if (!ReadDoubleCompat(child, "z", z)) continue;
                if (!ReadDoubleCompat(child, "w", w)) continue;
                p->emplace_back(Vector4((Float)x,(Float)y,(Float)z,(Float)w));
            }
            return true;
        }
        case RsdFieldType::DynArrayFloat:
        {
            auto* p = reinterpret_cast<std::vector<Float>*>(base + f.offset);
            if (!el) return true;
            for (auto* child = el->FirstChildElement(); child; child = child->NextSiblingElement())
            {
                if (const char* v = GetScalarTextCompat(child))
                    p->push_back((Float)std::atof(v));
            }
            return true;
        }
        case RsdFieldType::DynArrayUInt:
        {
            auto* p = reinterpret_cast<std::vector<UInt>*>(base + f.offset);
            if (!el) return true;
            for (auto* child = el->FirstChildElement(); child; child = child->NextSiblingElement())
            {
                if (const char* v = GetScalarTextCompat(child))
                    p->push_back((UInt)std::strtoul(v, nullptr, 10));
            }
            return true;
        }
        case RsdFieldType::MapStringVector4:
        {
            auto* p = reinterpret_cast<std::map<std::string, Vector4>*>(base + f.offset);
            if (!el) return true;
            for (auto* it = el->FirstChildElement("item"); it; it = it->NextSiblingElement("item"))
            {
                const char* key = it->Attribute("key");
                if (!key) key = it->Attribute("name");
                if (!key) continue;
                double x=0,y=0,z=0,w=0;
                if (!ReadDoubleCompat(it, "x", x)) continue;
                if (!ReadDoubleCompat(it, "y", y)) continue;
                if (!ReadDoubleCompat(it, "z", z)) continue;
                if (!ReadDoubleCompat(it, "w", w)) continue;
                (*p)[std::string(key)] = Vector4((Float)x,(Float)y,(Float)z,(Float)w);
            }
            return true;
        }
        case RsdFieldType::MapStringString:
        case RsdFieldType::MapStringRawReference:
        {
            auto* p = reinterpret_cast<std::map<std::string, std::string>*>(base + f.offset);
            if (!el) return true;
            for (auto* it = el->FirstChildElement("item"); it; it = it->NextSiblingElement("item"))
            {
                const char* key = it->Attribute("key");
                if (!key) key = it->Attribute("name");
                if (!key) continue;
                if (const char* v = GetScalarTextCompat(it))
                    (*p)[std::string(key)] = std::string(v);
            }
            return true;
        }
        case RsdFieldType::MapStringFloat:
        {
            auto* p = reinterpret_cast<std::map<std::string, Float>*>(base + f.offset);
            if (!el) return true;
            for (auto* it = el->FirstChildElement("item"); it; it = it->NextSiblingElement("item"))
            {
                const char* key = it->Attribute("key");
                if (!key) key = it->Attribute("name");
                if (!key) continue;
                if (const char* v = GetScalarTextCompat(it))
                    (*p)[std::string(key)] = (Float)std::atof(v);
            }
            return true;
        }
        case RsdFieldType::Unsupported:
			LOG_ERROR("Unsupported RSD field type for field '{0}'", f.name);
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

    bool ParseRsdObjectFromXmlElement(const tinyxml2::XMLElement* root, void* outBase, const RsdFieldDesc* fields, std::size_t fieldCount)
    {
        if (!root || !outBase || !fields || fieldCount == 0)
            return false;

        bool ok = true;
        for (std::size_t i = 0; i < fieldCount; i++)
        {
            if (!ParseFieldInto(outBase, fields[i], root))
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

    AssetBase* AssetManagerNew::GetAsset(const std::string& relative_file_path)
    {
        auto [it, inserted] = m_assets.try_emplace(relative_file_path, std::make_unique<AssetBase>());
        return it->second.get();
    }
}
