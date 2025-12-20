###############################################################################
# rsd_generate_one.cmake (single-file generator)
#
# This script generates ONE C++ header (.h) from ONE RSD schema file (.rsd).
#
# How it is executed:
#   CMake custom_command calls:
#     cmake -DINPUT=/path/to/foo.rsd -DOUTPUT=/path/to/foo.h -P rsd_generate_one.cmake
#
# Why it uses regex instead of an XML parser:
#   - keep the generator dependency-free (CMake-only)
#   - keep it fast and easy to run in "cmake -P" script mode
#   - the RSD XML is deliberately constrained/simple
#
# Output shape (simplified):
#   struct <class_name> {
#     static constexpr const char* kFileSuffix = "<file_suffix>";
#     ...fields...
#     static const std::array<RsdFieldDesc, N> kFields;
#   };
#
#   inline const std::array<RsdFieldDesc, N> <class_name>::kFields = {{
#     {"field_name", RsdFieldType::<...>, offsetof(<class_name>, field_name)},
#     ...
#   }};
#
# The generated `kFields` drives the engine-side generic loader:
#   AssetManager::LoadAndParseRsdFile(...) iterates kFields and writes into the
#   struct by offset (byte offset from offsetof).
###############################################################################

# Validate required variables from -DINPUT=... -DOUTPUT=...
if(NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "rsd_generate_one.cmake requires -DINPUT=... -DOUTPUT=...")
endif()

# Some shells/generators preserve quotes in -DINPUT="C:/a/b", which breaks file(READ).
# We strip a single pair of surrounding quotes if present.
set(_in "${INPUT}")   # raw INPUT (possibly quoted)
set(_out "${OUTPUT}") # raw OUTPUT (possibly quoted)
string(REGEX REPLACE "^\"(.*)\"$" "\\1" _in "${_in}")   # "X" -> X
string(REGEX REPLACE "^\"(.*)\"$" "\\1" _out "${_out}") # "X" -> X

# Read the whole RSD schema as plain text.
file(READ "${_in}" _rsd_text)

# RSD (XML) 格式：
# <rsd class_name="CameraRSD" file_suffix=".camera">
#   <field name="position" type="Vector3" />
# </rsd>

# Extract required root attributes (class_name and file_suffix) via regex.
string(REGEX MATCH "class_name[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _m_class "${_rsd_text}")
set(_class_name "${CMAKE_MATCH_1}") # Generated C++ struct name

string(REGEX MATCH "file_suffix[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _m_suffix "${_rsd_text}")
set(_file_suffix "${CMAKE_MATCH_1}") # File extension mapped to this schema (e.g. ".camera")

if(_class_name STREQUAL "")
    message(FATAL_ERROR "RSD missing required attribute: class_name (${_in})")
endif()
if(_file_suffix STREQUAL "")
    message(FATAL_ERROR "RSD missing required attribute: file_suffix (${_in})")
endif()

# CMake lists are ';' separated. To keep each `<field ...>` tag as ONE list element,
# we first escape ';' in the raw text, then run MATCHALL, then restore ';' per tag.
set(_rsd_text_safe "${_rsd_text}") # Work on a copy so we can safely pre-process characters without losing the original text.
string(REPLACE ";" "__SC__" _rsd_text_safe "${_rsd_text_safe}") # Escape ';' because CMake lists use ';' as a separator (prevents tags from being split).
string(REGEX MATCHALL "<field[^>]*>" _field_tags "${_rsd_text_safe}") # Collect all "<field ...>" tags into a CMake list (_field_tags).

set(_field_lines "") # Accumulator for generated C++ member declarations (e.g. "Vector3 position;").
set(_field_desc_lines "") # Accumulator for generated RsdFieldDesc initializers used to build kFields.
set(_need_map FALSE) # Whether any field requires <map> (kept for future conditional includes).
set(_need_set FALSE) # Whether any field requires <set> (kept for future conditional includes).
set(_need_vector FALSE) # Whether any field requires <vector> (kept for future conditional includes).
set(_need_string FALSE) # Whether any field requires <string> (kept for future conditional includes).
set(_need_nlohmann_json FALSE) # Whether any field uses nlohmann::json (legacy/optional).

function(_trim _in _out) # Trim leading/trailing whitespace (helper used by type parsing and attribute splitting)
    set(_s "${_in}") # Copy input into a local variable (CMake function scope).
    string(REGEX REPLACE "^[ \t\r\n]+" "" _s "${_s}") # Remove leading spaces/tabs/newlines.
    string(REGEX REPLACE "[ \t\r\n]+$" "" _s "${_s}") # Remove trailing spaces/tabs/newlines.
    set(${_out} "${_s}" PARENT_SCOPE) # Return the trimmed string to the caller via an output variable name.
endfunction() # End helper function

function(_split_toplevel_commas _in _out_list)
    # Split by top-level commas (ignore commas inside '<...>').
    # Example: "String, Vector4" -> ["String", "Vector4"]
    #          "Map<String, Vector4>" must NOT be split inside "<...>".
    set(s "${_in}")
    set(depth 0)
    set(cur "")
    set(out "")
    string(LENGTH "${s}" n)
    if(n EQUAL 0)
        set(${_out_list} "" PARENT_SCOPE)
        return()
    endif()
    math(EXPR last "${n}-1")
    foreach(i RANGE 0 ${last})
        string(SUBSTRING "${s}" ${i} 1 c)
        if(c STREQUAL "<")
            math(EXPR depth "${depth}+1")
        elseif(c STREQUAL ">")
            math(EXPR depth "${depth}-1")
        endif()

        if(c STREQUAL "," AND depth EQUAL 0)
            _trim("${cur}" cur_t)
            list(APPEND out "${cur_t}")
            set(cur "")
        else()
            set(cur "${cur}${c}")
        endif()
    endforeach()

    _trim("${cur}" cur_t)
    if(NOT cur_t STREQUAL "")
        list(APPEND out "${cur_t}")
    endif()
    set(${_out_list} "${out}" PARENT_SCOPE)
endfunction()

function(_cpp_type _spec _out_type _out_suffix)
    # Map an RSD type string (e.g. "Vector3", "Map<String, Float>") to:
    # - a C++ type name (t)
    # - an optional name suffix (sfx), currently only used for C-style arrays: "[N]"
    _trim("${_spec}" spec)

    if(spec STREQUAL "Float")
        set(t "Float")
        set(sfx "")
    elseif(spec STREQUAL "Double")
        set(t "Double")
        set(sfx "")
    elseif(spec STREQUAL "Int")
        set(t "Int")
        set(sfx "")
    elseif(spec STREQUAL "UInt")
        set(t "UInt")
        set(sfx "")
    elseif(spec STREQUAL "Bool")
        set(t "Bool")
        set(sfx "")
    elseif(spec STREQUAL "String")
        set(t "std::string")
        set(sfx "")
        set(_need_string TRUE PARENT_SCOPE)
    elseif(spec STREQUAL "Vector3")
        set(t "Vector3")
        set(sfx "")
    elseif(spec STREQUAL "Vector4")
        set(t "Vector4")
        set(sfx "")
    elseif(spec STREQUAL "Json")
        set(t "nlohmann::json")
        set(sfx "")
        set(_need_nlohmann_json TRUE PARENT_SCOPE)
    elseif(spec STREQUAL "Xml")
        set(t "std::string")
        set(sfx "")
        set(_need_string TRUE PARENT_SCOPE)
    elseif(spec STREQUAL "TriangleList")
        # TODO: 后续可升级为 enum/强类型；目前先保证可编译
        set(t "std::string")
        set(sfx "")
        set(_need_string TRUE PARENT_SCOPE)
    else()
        string(FIND "${spec}" "<" lt)
        string(FIND "${spec}" ">" gt REVERSE)
        if(lt GREATER -1 AND gt GREATER -1 AND gt GREATER lt)
            string(SUBSTRING "${spec}" 0 ${lt} base)
            math(EXPR inside_len "${gt}-${lt}-1")
            math(EXPR inside_off "${lt}+1")
            string(SUBSTRING "${spec}" ${inside_off} ${inside_len} inside)
            _trim("${base}" base)
            _trim("${inside}" inside)

            if(base STREQUAL "DynamicArray")
                _cpp_type("${inside}" inner_t inner_sfx)
                set(t "std::vector<${inner_t}>")
                set(sfx "")
                set(_need_vector TRUE PARENT_SCOPE)
            elseif(base STREQUAL "Set")
                _cpp_type("${inside}" inner_t inner_sfx)
                set(t "std::set<${inner_t}>")
                set(sfx "")
                set(_need_set TRUE PARENT_SCOPE)
            elseif(base STREQUAL "Map")
                _split_toplevel_commas("${inside}" args)
                list(LENGTH args argc)
                if(NOT argc EQUAL 2)
                    message(FATAL_ERROR "Map<K,V> expects 2 args: ${spec}")
                endif()
                list(GET args 0 kspec)
                list(GET args 1 vspec)
                _cpp_type("${kspec}" kt ksfx)
                _cpp_type("${vspec}" vt vsfx)
                set(t "std::map<${kt}, ${vt}>")
                set(sfx "")
                set(_need_map TRUE PARENT_SCOPE)
            elseif(base STREQUAL "StaticArray")
                _split_toplevel_commas("${inside}" args)
                list(LENGTH args argc)
                if(NOT argc EQUAL 2)
                    message(FATAL_ERROR "StaticArray<T,N> expects 2 args: ${spec}")
                endif()
                list(GET args 0 espec)
                list(GET args 1 nstr)
                _trim("${nstr}" nstr_t)
                if(NOT nstr_t MATCHES "^[0-9]+$")
                    message(FATAL_ERROR "StaticArray size must be integer: ${spec}")
                endif()
                _cpp_type("${espec}" et esfx)
                set(t "${et}")
                set(sfx "[${nstr_t}]")
            elseif(base STREQUAL "AssetReference")
                # TODO: 后续可生成 AssetReference<T>；目前先降级成路径字符串
                set(t "std::string")
                set(sfx "")
                set(_need_string TRUE PARENT_SCOPE)
            else()
                set(t "${spec}")
                set(sfx "")
            endif()
        else()
            set(t "${spec}")
            set(sfx "")
        endif()
    endif()

    set(${_out_type} "${t}" PARENT_SCOPE)
    set(${_out_suffix} "${sfx}" PARENT_SCOPE)
endfunction()

function(_field_type_enum type_spec out_enum)
    # Map the same RSD type string to a runtime enum (RsdFieldType::X).
    # This enum is consumed by the engine-side generic XML parser switch.
    set(t "${type_spec}")
    if(t STREQUAL "String")
        set(e "RsdFieldType::String")
    elseif(t STREQUAL "Bool")
        set(e "RsdFieldType::BoolValue")
    elseif(t STREQUAL "Int")
        set(e "RsdFieldType::IntValue")
    elseif(t STREQUAL "UInt")
        set(e "RsdFieldType::UIntValue")
    elseif(t STREQUAL "Float")
        set(e "RsdFieldType::FloatValue")
    elseif(t STREQUAL "Vector3")
        set(e "RsdFieldType::Vector3")
    elseif(t STREQUAL "Vector4")
        set(e "RsdFieldType::Vector4")
    elseif(t STREQUAL "DynamicArray<Xml>")
        set(e "RsdFieldType::DynArrayXml")
    elseif(t STREQUAL "DynamicArray<Float>")
        set(e "RsdFieldType::DynArrayFloat")
    elseif(t STREQUAL "DynamicArray<UInt>")
        set(e "RsdFieldType::DynArrayUInt")
    elseif(t STREQUAL "Map<String, Vector4>")
        set(e "RsdFieldType::MapStringVector4")
    elseif(t STREQUAL "Map<String, String>")
        set(e "RsdFieldType::MapStringString")
    elseif(t STREQUAL "Map<String, Float>")
        set(e "RsdFieldType::MapStringFloat")
    else()
        set(e "RsdFieldType::Unsupported")
    endif()
    set(${out_enum} "${e}" PARENT_SCOPE)
endfunction()

foreach(tag IN LISTS _field_tags)
    # Restore escaped ';' for this tag so regex can read it correctly.
    string(REPLACE "__SC__" ";" tag "${tag}")

    string(REGEX MATCH "name[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _mn "${tag}")
    set(k "${CMAKE_MATCH_1}")
    string(REGEX MATCH "type[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _mt "${tag}")
    set(v "${CMAKE_MATCH_1}")

    if(k STREQUAL "" OR v STREQUAL "")
        message(FATAL_ERROR "Invalid <field> tag (missing name/type): ${tag} in ${_in}")
    endif()

    # Decode XML entities because we don't use a real XML parser here.
    string(REPLACE "&lt;" "<" v "${v}")
    string(REPLACE "&gt;" ">" v "${v}")
    string(REPLACE "&amp;" "&" v "${v}")

    # Optional "generic-like" attribute form to avoid writing &lt; / &gt; inside XML attributes.
    # Examples:
    #   <field name="x" type="Map" key="String" value="Vector4" />
    #   <field name="x" type="DynamicArray" element="Float" />
    #   <field name="x" type="Set" element="String" />
    #   <field name="x" type="StaticArray" element="Float" size="16" />
    #   <field name="x" type="AssetReference" target="MaterialRSD" />
    #
    # We normalize both the old and the new form into a canonical spec string like:
    #   Map<String, Vector4> / DynamicArray<Float> / AssetReference<MaterialRSD>
    # so the rest of the generator stays unchanged.
    set(v_full "${v}")
    if(v STREQUAL "Map")
        string(REGEX MATCH "key[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _mk "${tag}")
        set(k_spec "${CMAKE_MATCH_1}")
        string(REGEX MATCH "value[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _mv "${tag}")
        set(v_spec "${CMAKE_MATCH_1}")
        if(k_spec STREQUAL "" OR v_spec STREQUAL "")
            message(FATAL_ERROR "Map field requires key/value attributes: ${tag} in ${_in}")
        endif()
        string(REPLACE "&lt;" "<" k_spec "${k_spec}")
        string(REPLACE "&gt;" ">" k_spec "${k_spec}")
        string(REPLACE "&amp;" "&" k_spec "${k_spec}")
        string(REPLACE "&lt;" "<" v_spec "${v_spec}")
        string(REPLACE "&gt;" ">" v_spec "${v_spec}")
        string(REPLACE "&amp;" "&" v_spec "${v_spec}")
        set(v_full "Map<${k_spec}, ${v_spec}>")
    elseif(v STREQUAL "DynamicArray" OR v STREQUAL "Set")
        string(REGEX MATCH "element[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _me "${tag}")
        set(e_spec "${CMAKE_MATCH_1}")
        if(e_spec STREQUAL "")
            message(FATAL_ERROR "${v} field requires element attribute: ${tag} in ${_in}")
        endif()
        string(REPLACE "&lt;" "<" e_spec "${e_spec}")
        string(REPLACE "&gt;" ">" e_spec "${e_spec}")
        string(REPLACE "&amp;" "&" e_spec "${e_spec}")
        set(v_full "${v}<${e_spec}>")
    elseif(v STREQUAL "StaticArray")
        string(REGEX MATCH "element[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _me "${tag}")
        set(e_spec "${CMAKE_MATCH_1}")
        string(REGEX MATCH "size[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _ms "${tag}")
        set(n_spec "${CMAKE_MATCH_1}")
        if(e_spec STREQUAL "" OR n_spec STREQUAL "")
            message(FATAL_ERROR "StaticArray field requires element/size attributes: ${tag} in ${_in}")
        endif()
        string(REPLACE "&lt;" "<" e_spec "${e_spec}")
        string(REPLACE "&gt;" ">" e_spec "${e_spec}")
        string(REPLACE "&amp;" "&" e_spec "${e_spec}")
        set(v_full "StaticArray<${e_spec}, ${n_spec}>")
    elseif(v STREQUAL "AssetReference")
        string(REGEX MATCH "target[ \t\r\n]*=[ \t\r\n]*\"([^\"]+)\"" _mtgt "${tag}")
        set(t_spec "${CMAKE_MATCH_1}")
        if(t_spec STREQUAL "")
            message(FATAL_ERROR "AssetReference field requires target attribute: ${tag} in ${_in}")
        endif()
        string(REPLACE "&lt;" "<" t_spec "${t_spec}")
        string(REPLACE "&gt;" ">" t_spec "${t_spec}")
        string(REPLACE "&amp;" "&" t_spec "${t_spec}")
        set(v_full "AssetReference<${t_spec}>")
    endif()

    # Produce the C++ member line for this field (e.g. "Vector3 position;").
    _cpp_type("${v_full}" cpp_t cpp_sfx)
    set(_field_lines "${_field_lines}    ${cpp_t} ${k}${cpp_sfx};\n")

    # Produce the reflection descriptor line used for runtime parsing by offset.
    _field_type_enum("${v_full}" field_enum)
    set(_field_desc_lines "${_field_desc_lines}        {\"${k}\", ${field_enum}, offsetof(${_class_name}, ${k})},\n")
endforeach()

list(LENGTH _field_tags _field_count) # Count how many fields we have.  (_field_tags is a CMake list of "<field ...>" tags).

get_filename_component(_rsd_name "${_in}" NAME) # Get the name of the RSD file (without path).
set(_header "") # Accumulator for the generated header content.
string(APPEND _header "// Auto-generated by precompile (RSD -> .h). DO NOT EDIT.\n")
string(APPEND _header "// Source: ${_rsd_name}\n\n")
string(APPEND _header "#pragma once\n\n") # Standard C++ include guard.
# Standard library includes (only emit what we actually use).
# Note: <array> is always required because kFields is a std::array.
# Note: <cstddef> is required for offsetof / std::size_t in generated code.
if(_need_string)
    string(APPEND _header "#include <string>\n")
endif()
if(_need_vector)
    string(APPEND _header "#include <vector>\n")
endif()
if(_need_map)
    string(APPEND _header "#include <map>\n")
endif()
if(_need_set)
    string(APPEND _header "#include <set>\n")
endif()
string(APPEND _header "#include <array>\n")
string(APPEND _header "#include <cstddef>\n")

if(_need_nlohmann_json)
    string(APPEND _header "#include <nlohmann/json.hpp>\n")
endif()
string(APPEND _header "#include \"base/dolas_base.h\"\n")
string(APPEND _header "#include \"core/dolas_math.h\"\n")
string(APPEND _header "#include \"common/rsd_field.h\"\n\n")

string(APPEND _header "namespace Dolas {\n\n")
string(APPEND _header "struct ${_class_name}\n{\n")
string(APPEND _header "    static constexpr const char* kFileSuffix = \"${_file_suffix}\";\n")
string(APPEND _header "${_field_lines}")
string(APPEND _header "\n    static const std::array<RsdFieldDesc, ${_field_count}> kFields;\n")
string(APPEND _header "};\n\n")
string(APPEND _header "inline const std::array<RsdFieldDesc, ${_field_count}> ${_class_name}::kFields = {{\n${_field_desc_lines}}};\n\n")
string(APPEND _header "} // namespace Dolas\n")

# Write only if content differs to avoid unnecessary rebuilds in incremental workflows.
set(_old "")
if(EXISTS "${_out}")
    file(READ "${_out}" _old)
endif()
if(NOT _old STREQUAL _header)
    get_filename_component(_out_dir "${_out}" DIRECTORY)
    file(MAKE_DIRECTORY "${_out_dir}")
    file(WRITE "${_out}" "${_header}")
endif()


