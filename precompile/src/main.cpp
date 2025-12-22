#include <tinyxml2.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

struct Field
{
    std::string name;
    std::string typeSpec; // canonical spec string, e.g. "Vector3", "Map<String, Vector4>", "DynamicArray<Xml>"
};

struct EnumValue
{
    std::string name;    // C++ enumerator identifier
    std::string display; // UI label (optional)
    std::string alias;   // legacy/alternate text (optional)
    uint64_t value = 0;  // numeric value (assigned by order if not specified)
};

struct EnumDef
{
    std::string name;                 // enum class name
    std::string underlying = "UInt";  // currently only support UInt
    std::vector<EnumValue> values;
};

struct Schema
{
    std::string rsdFileName; // e.g. "camera.rsd"
    std::string stem;        // e.g. "camera"
    std::string className;   // e.g. "CameraRSD"
    std::string fileSuffix;  // e.g. ".camera"
    std::vector<Field> fields;

    // Optional enums declared in the same .rsd file (rsd_file format).
    std::vector<EnumDef> enums;
};

static inline std::string Trim(std::string s)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!s.empty() && isSpace((unsigned char)s.front())) s.erase(s.begin());
    while (!s.empty() && isSpace((unsigned char)s.back())) s.pop_back();
    return s;
}

static std::vector<std::string> SplitTopLevelCommas(const std::string& s)
{
    std::vector<std::string> out;
    int depth = 0;
    std::string cur;
    for (char c : s)
    {
        if (c == '<') depth++;
        else if (c == '>') depth--;

        if (c == ',' && depth == 0)
        {
            out.push_back(Trim(cur));
            cur.clear();
        }
        else
        {
            cur.push_back(c);
        }
    }
    cur = Trim(cur);
    if (!cur.empty()) out.push_back(cur);
    return out;
}

struct CppType
{
    std::string type;   // e.g. "std::string", "Vector3", "std::map<...>"
    std::string suffix; // e.g. "[16]" for C-style arrays
};

struct IncludeNeeds
{
    bool needString = false;
    bool needVector = false;
    bool needMap = false;
    bool needSet = false;
    bool needNlohmannJson = false;
};

static CppType CppTypeForSpec(const std::string& specIn, IncludeNeeds& needs);

static CppType CppTypeForSpec(const std::string& specIn, IncludeNeeds& needs)
{
    const std::string spec = Trim(specIn);

    if (spec == "Float") return { "Float", "" };
    if (spec == "Double") return { "Double", "" };
    if (spec == "Int") return { "Int", "" };
    if (spec == "UInt") return { "UInt", "" };
    if (spec == "Bool") return { "Bool", "" };
    if (spec == "Vector3") return { "Vector3", "" };
    if (spec == "Vector4") return { "Vector4", "" };

    if (spec == "String" || spec == "Xml" || spec == "TriangleList" || spec.rfind("AssetReference", 0) == 0)
    {
        needs.needString = true;
        return { "std::string", "" };
    }

    if (spec == "Json")
    {
        needs.needNlohmannJson = true;
        return { "nlohmann::json", "" };
    }

    const auto lt = spec.find('<');
    const auto gt = spec.rfind('>');
    if (lt != std::string::npos && gt != std::string::npos && gt > lt)
    {
        const std::string base = Trim(spec.substr(0, lt));
        const std::string inside = Trim(spec.substr(lt + 1, gt - lt - 1));

        if (base == "Enum")
        {
            // Enum<CameraPerspectiveType> -> member type is CameraPerspectiveType
            return { inside, "" };
        }
        if (base == "DynamicArray")
        {
            CppType inner = CppTypeForSpec(inside, needs);
            needs.needVector = true;
            return { "std::vector<" + inner.type + ">", "" };
        }
        if (base == "Set")
        {
            CppType inner = CppTypeForSpec(inside, needs);
            needs.needSet = true;
            return { "std::set<" + inner.type + ">", "" };
        }
        if (base == "Map")
        {
            const auto args = SplitTopLevelCommas(inside);
            if (args.size() != 2) throw std::runtime_error("Map<K,V> expects 2 args: " + spec);
            CppType kt = CppTypeForSpec(args[0], needs);
            CppType vt = CppTypeForSpec(args[1], needs);
            needs.needMap = true;
            return { "std::map<" + kt.type + ", " + vt.type + ">", "" };
        }
        if (base == "StaticArray")
        {
            const auto args = SplitTopLevelCommas(inside);
            if (args.size() != 2) throw std::runtime_error("StaticArray<T,N> expects 2 args: " + spec);
            const std::string elemSpec = Trim(args[0]);
            const std::string nStr = Trim(args[1]);
            if (nStr.empty() || !std::all_of(nStr.begin(), nStr.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
                throw std::runtime_error("StaticArray size must be integer: " + spec);
            CppType et = CppTypeForSpec(elemSpec, needs);
            return { et.type, "[" + nStr + "]" };
        }
    }

    // Fallback: treat as a raw C++ type name.
    return { spec, "" };
}

static std::string FieldEnumForSpec(const std::string& typeSpecIn)
{
    const std::string t = Trim(typeSpecIn);
    if (t.rfind("Enum<", 0) == 0) return "RsdFieldType::EnumUInt";
    // AssetReference<T> / RawReference are represented as string in C++.
    if (t.rfind("AssetReference<", 0) == 0) return "RsdFieldType::String";
    if (t == "RawReference") return "RsdFieldType::String";
    if (t == "String") return "RsdFieldType::String";
    if (t == "Bool") return "RsdFieldType::BoolValue";
    if (t == "Int") return "RsdFieldType::IntValue";
    if (t == "UInt") return "RsdFieldType::UIntValue";
    if (t == "Float") return "RsdFieldType::FloatValue";
    if (t == "Vector3") return "RsdFieldType::Vector3";
    if (t == "Vector4") return "RsdFieldType::Vector4";
    // Arrays
    if (t == "DynamicArray<RawReference>") return "RsdFieldType::DynArrayRawReference";
    if (t == "DynamicArray<String>" || t.rfind("DynamicArray<AssetReference<", 0) == 0) return "RsdFieldType::DynArrayString";
    if (t == "DynamicArray<Vector3>") return "RsdFieldType::DynArrayVector3";
    if (t == "DynamicArray<Vector4>") return "RsdFieldType::DynArrayVector4";
    if (t == "DynamicArray<Float>") return "RsdFieldType::DynArrayFloat";
    if (t == "DynamicArray<UInt>") return "RsdFieldType::DynArrayUInt";
    if (t == "Map<String, Vector4>") return "RsdFieldType::MapStringVector4";
    if (t == "Map<String, String>") return "RsdFieldType::MapStringString";
    if (t == "Map<String, Float>") return "RsdFieldType::MapStringFloat";
    return "RsdFieldType::Unsupported";
}

static std::string CanonicalizeTypeSpec(const tinyxml2::XMLElement* fieldEl)
{
    const char* typeAttr = fieldEl->Attribute("type");
    if (!typeAttr) throw std::runtime_error("<field> missing required attribute: type");
    std::string type = Trim(typeAttr);

    // New attribute form (avoid XML entities in attributes):
    //   Map: key/value
    //   DynamicArray/Set: element
    //   StaticArray: element/size
    //   AssetReference: target
    if (type == "Map")
    {
        const char* k = fieldEl->Attribute("key");
        const char* v = fieldEl->Attribute("value");
        if (!k || !v) throw std::runtime_error("Map field requires key/value attributes");
        return "Map<" + Trim(k) + ", " + Trim(v) + ">";
    }
    if (type == "DynamicArray" || type == "Set")
    {
        const char* e = fieldEl->Attribute("element");
        if (!e) throw std::runtime_error(type + " field requires element attribute");
        std::string elem = Trim(e);
        // Support attribute form for nested AssetReference to avoid XML entities:
        //   <field type="DynamicArray" element="AssetReference" target="EntityRSD" />
        if (elem == "AssetReference")
        {
            const char* t = fieldEl->Attribute("target");
            if (!t) throw std::runtime_error(type + " element=AssetReference requires target attribute");
            elem = "AssetReference<" + Trim(t) + ">";
        }
        return type + "<" + elem + ">";
    }
    if (type == "StaticArray")
    {
        const char* e = fieldEl->Attribute("element");
        const char* n = fieldEl->Attribute("size");
        if (!n) n = fieldEl->Attribute("count");
        if (!n) n = fieldEl->Attribute("n");
        if (!e || !n) throw std::runtime_error("StaticArray field requires element/size attributes");
        return "StaticArray<" + Trim(e) + ", " + Trim(n) + ">";
    }
    if (type == "AssetReference")
    {
        const char* t = fieldEl->Attribute("target");
        if (!t) throw std::runtime_error("AssetReference field requires target attribute");
        return "AssetReference<" + Trim(t) + ">";
    }

    if (type == "RawReference")
    {
        return "RawReference";
    }

    // Old form is already in type attribute (tinyxml2 will decode &lt; &gt;).
    return type;
}

static EnumDef LoadEnum(const tinyxml2::XMLElement* enumEl)
{
    const char* className = enumEl->Attribute("class_name");
    if (!className || !*className) throw std::runtime_error("<enum> missing class_name");

    EnumDef e;
    e.name = Trim(className);

    if (const char* u = enumEl->Attribute("underlying"); u && *u)
        e.underlying = Trim(u);

    if (e.underlying != "UInt")
        throw std::runtime_error("Only underlying=\"UInt\" is supported for enum currently: " + e.name);

    uint64_t nextValue = 0;
    for (auto* v = enumEl->FirstChildElement("value"); v; v = v->NextSiblingElement("value"))
    {
        const char* n = v->Attribute("name");
        if (!n || !*n) throw std::runtime_error("<value> missing name in enum: " + e.name);

        EnumValue ev;
        ev.name = Trim(n);

        if (const char* d = v->Attribute("display"); d && *d) ev.display = Trim(d);
        if (const char* a = v->Attribute("alias"); a && *a) ev.alias = Trim(a);

        if (const char* val = v->Attribute("value"); val && *val)
        {
            ev.value = std::strtoull(val, nullptr, 10);
            nextValue = ev.value + 1;
        }
        else
        {
            ev.value = nextValue++;
        }

        e.values.push_back(std::move(ev));
    }

    if (e.values.empty())
        throw std::runtime_error("Enum has no <value> entries: " + e.name);

    return e;
}

static Schema LoadSchema(const fs::path& rsdPath)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(rsdPath.string().c_str()) != tinyxml2::XML_SUCCESS)
        throw std::runtime_error("Failed to load RSD: " + rsdPath.string());

    const tinyxml2::XMLElement* root = doc.RootElement();
    if (!root)
        throw std::runtime_error("RSD XML has no root element: " + rsdPath.string());

    // Supported formats:
    // - legacy:   <rsd class_name="X" file_suffix=".x">...</rsd>
    // - extended: <rsd_file><enum ...>...</enum><rsd ...>...</rsd></rsd_file>
    const tinyxml2::XMLElement* rsdRoot = nullptr;
    std::vector<EnumDef> enums;

    const std::string rootName = root->Name();
    if (rootName == "rsd")
    {
        rsdRoot = root;
    }
    else if (rootName == "rsd_file")
    {
        for (auto* e = root->FirstChildElement("enum"); e; e = e->NextSiblingElement("enum"))
            enums.push_back(LoadEnum(e));

        rsdRoot = root->FirstChildElement("rsd");
        if (!rsdRoot)
            throw std::runtime_error("<rsd_file> missing <rsd> element: " + rsdPath.string());
    }
    else
    {
        throw std::runtime_error("RSD root must be <rsd> or <rsd_file>: " + rsdPath.string());
    }

    const char* className = rsdRoot->Attribute("class_name");
    const char* fileSuffix = rsdRoot->Attribute("file_suffix");
    if (!className || !*className) throw std::runtime_error("RSD missing class_name: " + rsdPath.string());
    if (!fileSuffix || !*fileSuffix) throw std::runtime_error("RSD missing file_suffix: " + rsdPath.string());

    Schema s;
    s.rsdFileName = rsdPath.filename().string();
    s.stem = rsdPath.stem().string();
    s.className = Trim(className);
    s.fileSuffix = Trim(fileSuffix);
    s.enums = std::move(enums);

    std::unordered_set<std::string> enumNames;
    for (const auto& e : s.enums) enumNames.insert(e.name);

    for (auto* f = rsdRoot->FirstChildElement("field"); f; f = f->NextSiblingElement("field"))
    {
        const char* name = f->Attribute("name");
        if (!name || !*name) throw std::runtime_error("RSD <field> missing name: " + rsdPath.string());

        Field field;
        field.name = Trim(name);
        field.typeSpec = CanonicalizeTypeSpec(f);

        // If a field refers to a declared enum, normalize to Enum<...>
        if (enumNames.contains(field.typeSpec))
            field.typeSpec = "Enum<" + field.typeSpec + ">";

        s.fields.push_back(std::move(field));
    }

    return s;
}

static std::string ReadAllText(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    if (!in) return {};
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void WriteAllTextIfChanged(const fs::path& p, const std::string& text)
{
    const std::string old = ReadAllText(p);
    if (old == text) return;
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("Failed to write: " + p.string());
    out << text;
}

static std::string GenerateHeader(const Schema& s)
{
    IncludeNeeds needs;

    std::ostringstream fieldLines;
    std::ostringstream descLines;

    // Enum declarations + enum mapping tables (only for rsd_file format).
    std::ostringstream enumDecls;
    std::ostringstream enumTables;
    bool hasEnums = !s.enums.empty();

    auto escapeForCxx = [](const std::string& str) {
        std::string out;
        out.reserve(str.size());
        for (char c : str)
        {
            if (c == '\\' || c == '"') out.push_back('\\');
            out.push_back(c);
        }
        return out;
    };

    if (hasEnums)
    {
        for (const auto& e : s.enums)
        {
            enumDecls << "enum class " << e.name << " : " << e.underlying << "\n{\n";
            for (const auto& v : e.values)
                enumDecls << "    " << v.name << " = " << v.value << ",\n";
            enumDecls << "};\n\n";

            enumTables << "inline const std::array<RsdEnumItemDesc, " << e.values.size() << "> kEnum_" << e.name << " = {{\n";
            for (const auto& v : e.values)
            {
                enumTables << "    {\""
                           << escapeForCxx(v.name) << "\", \""
                           << escapeForCxx(v.display) << "\", \""
                           << escapeForCxx(v.alias) << "\", "
                           << v.value << "},\n";
            }
            enumTables << "}};\n\n";
        }
    }

    for (const auto& f : s.fields)
    {
        CppType ct = CppTypeForSpec(f.typeSpec, needs);
        fieldLines << "    " << ct.type << " " << f.name << ct.suffix << ";\n";

        const std::string e = FieldEnumForSpec(f.typeSpec);
        if (e == "RsdFieldType::EnumUInt")
        {
            // typeSpec is "Enum<EnumName>"
            const auto lt = f.typeSpec.find('<');
            const auto gt = f.typeSpec.rfind('>');
            const std::string enumName = (lt != std::string::npos && gt != std::string::npos && gt > lt)
                ? Trim(f.typeSpec.substr(lt + 1, gt - lt - 1))
                : std::string();
            descLines << "        {\"" << f.name << "\", " << e << ", offsetof(" << s.className << ", " << f.name
                      << "), kEnum_" << enumName << ".data(), kEnum_" << enumName << ".size()},\n";
        }
        else
        {
            descLines << "        {\"" << f.name << "\", " << e << ", offsetof(" << s.className << ", " << f.name
                      << "), nullptr, 0},\n";
        }
    }

    std::ostringstream h;
    h << "// Auto-generated by precompile (RSD -> .h). DO NOT EDIT.\n";
    h << "// Source: " << s.rsdFileName << "\n\n";
    h << "#pragma once\n\n";

    if (needs.needString) h << "#include <string>\n";
    if (needs.needVector) h << "#include <vector>\n";
    if (needs.needMap) h << "#include <map>\n";
    if (needs.needSet) h << "#include <set>\n";
    h << "#include <array>\n";
    h << "#include <cstddef>\n";
    if (needs.needNlohmannJson) h << "#include <nlohmann/json.hpp>\n";

    h << "#include \"base/dolas_base.h\"\n";
    h << "#include \"core/dolas_math.h\"\n";
    h << "#include \"common/rsd_field.h\"\n\n";

    h << "namespace Dolas {\n\n";

    if (hasEnums)
    {
        h << enumDecls.str();
        h << enumTables.str();
    }

    h << "struct " << s.className << "\n{\n";
    h << "    static constexpr const char* kFileSuffix = \"" << s.fileSuffix << "\";\n";
    h << fieldLines.str();
    h << "\n    static const std::array<RsdFieldDesc, " << s.fields.size() << "> kFields;\n";
    h << "};\n\n";
    h << "inline const std::array<RsdFieldDesc, " << s.fields.size() << "> " << s.className << "::kFields = {{\n";
    h << descLines.str();
    h << "}};\n\n";
    h << "} // namespace Dolas\n";

    return h.str();
}

static void CleanupStaleHeaders(const fs::path& rsdDir, const fs::path& outDir, const std::unordered_set<std::string>& expectedStems)
{
    if (!fs::exists(outDir) || !fs::is_directory(outDir)) return;

    for (const auto& it : fs::directory_iterator(outDir))
    {
        if (!it.is_regular_file()) continue;
        const fs::path p = it.path();
        if (p.extension() != ".h") continue;

        const std::string stem = p.stem().string();
        if (stem == "rsd_field") continue; // safety

        if (!expectedStems.contains(stem))
        {
            std::cout << "[Precompile] remove stale header: " << p.string() << "\n";
            std::error_code ec;
            fs::remove(p, ec);
        }
    }
}

static void PrintUsage()
{
    std::cout << "Usage: Precompile --rsd-dir <path> --out-dir <path>\n";
}

int main(int argc, char** argv)
{
    try
    {
        fs::path rsdDir;
        fs::path outDir;

        for (int i = 1; i < argc; i++)
        {
            const std::string a = argv[i];
            if (a == "--rsd-dir" && i + 1 < argc) rsdDir = fs::path(argv[++i]);
            else if (a == "--out-dir" && i + 1 < argc) outDir = fs::path(argv[++i]);
            else if (a == "--help" || a == "-h")
            {
                PrintUsage();
                return 0;
            }
        }

        if (rsdDir.empty() || outDir.empty())
        {
            PrintUsage();
            return 2;
        }

        if (!fs::exists(rsdDir) || !fs::is_directory(rsdDir))
            throw std::runtime_error("rsd dir not found: " + rsdDir.string());

        fs::create_directories(outDir);

        std::vector<fs::path> rsdFiles;
        for (const auto& it : fs::directory_iterator(rsdDir))
        {
            if (!it.is_regular_file()) continue;
            if (it.path().extension() == ".rsd")
                rsdFiles.push_back(it.path());
        }
        std::sort(rsdFiles.begin(), rsdFiles.end());

        if (rsdFiles.empty())
            throw std::runtime_error("No .rsd files found under: " + rsdDir.string());

        std::unordered_set<std::string> expectedStems;
        size_t generated = 0;

        for (const auto& p : rsdFiles)
        {
            Schema s = LoadSchema(p);
            expectedStems.insert(s.stem);

            const fs::path outH = outDir / (s.stem + ".h");
            const std::string header = GenerateHeader(s);
            WriteAllTextIfChanged(outH, header);
            std::cout << "[Precompile] RSD -> H: " << s.rsdFileName << " -> " << outH.string() << "\n";
            generated++;
        }

        CleanupStaleHeaders(rsdDir, outDir, expectedStems);

        std::cout << "[Precompile] done. schemas=" << rsdFiles.size() << ", headers=" << generated << "\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Precompile] ERROR: " << e.what() << "\n";
        return 1;
    }
}


