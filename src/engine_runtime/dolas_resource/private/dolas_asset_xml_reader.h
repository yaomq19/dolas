#ifndef DOLAS_ASSET_XML_READER_H
#define DOLAS_ASSET_XML_READER_H

#include <charconv>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#include "dolas_asset_load_result.h"
#include "dolas_asset_ref.h"
#include "dolas_asset_schema.h"
#include "dolas_base.h"
#include "dolas_log_system_manager.h"
#include "dolas_math.h"
#include "tinyxml2.h"

namespace Dolas::Detail
{
    enum class AssetXmlErrorKind
    {
        Parse,
        Validation,
    };

    struct AssetXmlReadContext
    {
        [[nodiscard]] bool Fail(
            AssetXmlErrorKind error_kind,
            std::string field_path,
            std::string error_message)
        {
            if (message.empty())
            {
                kind = error_kind;
                path = std::move(field_path);
                message = std::move(error_message);
            }
            return false;
        }

        AssetXmlErrorKind kind{AssetXmlErrorKind::Parse};
        std::string path;
        std::string message;
    };

    template<class T>
    struct OptionalTraits
    {
        static constexpr bool kIsOptional = false;
    };

    template<class T>
    struct OptionalTraits<std::optional<T>>
    {
        static constexpr bool kIsOptional = true;
        using ValueType = T;
    };

    template<class T>
    struct VectorTraits
    {
        static constexpr bool kIsVector = false;
    };

    template<class T, class TAllocator>
    struct VectorTraits<std::vector<T, TAllocator>>
    {
        static constexpr bool kIsVector = true;
        using ValueType = T;
    };

    template<class T>
    struct StringMapTraits
    {
        static constexpr bool kIsStringMap = false;
    };

    template<class TValue, class TCompare, class TAllocator>
    struct StringMapTraits<std::map<std::string, TValue, TCompare, TAllocator>>
    {
        static constexpr bool kIsStringMap = true;
        using ValueType = TValue;
    };

    template<class T>
    struct AssetRefTraits
    {
        static constexpr bool kIsAssetRef = false;
    };

    template<class TAsset>
    struct AssetRefTraits<AssetRef<TAsset>>
    {
        static constexpr bool kIsAssetRef = true;
        using AssetType = TAsset;
    };

    [[nodiscard]] constexpr std::string_view Trim(std::string_view value) noexcept
    {
        constexpr std::string_view whitespace{" \t\r\n"};
        const std::size_t begin = value.find_first_not_of(whitespace);
        if (begin == std::string_view::npos)
        {
            return {};
        }
        const std::size_t end = value.find_last_not_of(whitespace);
        return value.substr(begin, end - begin + 1);
    }

    [[nodiscard]] constexpr char ToLowerAscii(char value) noexcept
    {
        if (value >= 'A' && value <= 'Z')
        {
            return static_cast<char>(value - 'A' + 'a');
        }
        return value;
    }

    [[nodiscard]] constexpr bool EqualsAsciiInsensitive(
        std::string_view left,
        std::string_view right) noexcept
    {
        if (left.size() != right.size())
        {
            return false;
        }
        for (std::size_t index = 0; index < left.size(); ++index)
        {
            if (ToLowerAscii(left[index]) != ToLowerAscii(right[index]))
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] inline std::optional<std::string_view> GetScalarText(
        const tinyxml2::XMLElement& element) noexcept
    {
        const char* text = element.GetText();
        if (text == nullptr)
        {
            return std::nullopt;
        }
        return Trim(text);
    }

    template<std::integral TValue>
    [[nodiscard]] bool ParseInteger(std::string_view text, TValue& output) noexcept
    {
        if (text.empty())
        {
            return false;
        }
        TValue value{};
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc{} || end != text.data() + text.size())
        {
            return false;
        }
        output = value;
        return true;
    }

    template<std::floating_point TValue>
    [[nodiscard]] bool ParseFloatingPoint(std::string_view text, TValue& output) noexcept
    {
        if (text.empty())
        {
            return false;
        }
        TValue value{};
        const auto [end, error] = std::from_chars(
            text.data(),
            text.data() + text.size(),
            value,
            std::chars_format::general);
        if (error != std::errc{} || end != text.data() + text.size() || !std::isfinite(value))
        {
            return false;
        }
        output = value;
        return true;
    }

    template<class TValue>
    [[nodiscard]] bool ReadValue(
        const tinyxml2::XMLElement& element,
        TValue& output,
        AssetXmlReadContext& context,
        const std::string& path);

    template<ReflectedAssetObject TObject>
    [[nodiscard]] bool ReadObject(
        const tinyxml2::XMLElement& element,
        TObject& output,
        AssetXmlReadContext& context,
        const std::string& path);

    template<class TValue>
    [[nodiscard]] std::optional<TValue> ReadConstructedValue(
        const tinyxml2::XMLElement& element,
        AssetXmlReadContext& context,
        const std::string& path)
    {
        if constexpr (AssetRefTraits<TValue>::kIsAssetRef)
        {
            using TAsset = typename AssetRefTraits<TValue>::AssetType;
            static_assert(AssetDescription<TAsset>);

            const auto text = GetScalarText(element);
            if (!text || text->empty())
            {
                context.Fail(AssetXmlErrorKind::Parse, path, "asset reference is empty");
                return std::nullopt;
            }

            auto asset_path = AssetPath::Parse(*text);
            if (!asset_path)
            {
                context.Fail(AssetXmlErrorKind::Parse, path, "asset reference path is invalid");
                return std::nullopt;
            }
            if (!asset_path->GetRelativePath().ends_with(TAsset::kFileSuffix))
            {
                context.Fail(AssetXmlErrorKind::Validation, path, "asset reference has the wrong file suffix");
                return std::nullopt;
            }
            return TValue{std::move(*asset_path)};
        }
        else if constexpr (std::same_as<TValue, RawAssetRef>)
        {
            const auto text = GetScalarText(element);
            if (!text || text->empty())
            {
                context.Fail(AssetXmlErrorKind::Parse, path, "raw asset reference is empty");
                return std::nullopt;
            }

            auto asset_path = AssetPath::Parse(*text);
            if (!asset_path)
            {
                context.Fail(AssetXmlErrorKind::Parse, path, "raw asset reference path is invalid");
                return std::nullopt;
            }
            return TValue{std::move(*asset_path)};
        }
        else
        {
            static_assert(std::default_initializable<TValue>);
            TValue value{};
            if (!ReadValue(element, value, context, path))
            {
                return std::nullopt;
            }
            return value;
        }
    }

    [[nodiscard]] inline bool ReadVectorComponent(
        const tinyxml2::XMLElement& element,
        const char* component_name,
        Float& output,
        AssetXmlReadContext& context,
        const std::string& path)
    {
        const char* attribute = element.Attribute(component_name);
        const tinyxml2::XMLElement* child = element.FirstChildElement(component_name);
        if (child != nullptr && child->NextSiblingElement(component_name) != nullptr)
        {
            return context.Fail(AssetXmlErrorKind::Parse, path, "vector component is duplicated");
        }
        if (attribute != nullptr && child != nullptr)
        {
            return context.Fail(AssetXmlErrorKind::Parse, path, "vector component uses two representations");
        }

        std::string_view text;
        if (attribute != nullptr)
        {
            text = Trim(attribute);
        }
        else if (child != nullptr)
        {
            const auto child_text = GetScalarText(*child);
            if (!child_text)
            {
                return context.Fail(AssetXmlErrorKind::Parse, path, "vector component is empty");
            }
            text = *child_text;
        }
        else
        {
            return context.Fail(AssetXmlErrorKind::Parse, path, "vector component is missing");
        }

        if (!ParseFloatingPoint(text, output))
        {
            return context.Fail(AssetXmlErrorKind::Parse, path, "vector component is not a finite number");
        }
        return true;
    }

    template<class TValue>
    [[nodiscard]] bool ReadValue(
        const tinyxml2::XMLElement& element,
        TValue& output,
        AssetXmlReadContext& context,
        const std::string& path)
    {
        if constexpr (std::same_as<TValue, std::string>)
        {
            const auto text = GetScalarText(element);
            if (!text)
            {
                return context.Fail(AssetXmlErrorKind::Parse, path, "string value is missing");
            }
            output.assign(*text);
            return true;
        }
        else if constexpr (std::same_as<TValue, bool>)
        {
            const auto text = GetScalarText(element);
            if (!text)
            {
                return context.Fail(AssetXmlErrorKind::Parse, path, "boolean value is missing");
            }
            if (EqualsAsciiInsensitive(*text, "true") || *text == "1")
            {
                output = true;
                return true;
            }
            if (EqualsAsciiInsensitive(*text, "false") || *text == "0")
            {
                output = false;
                return true;
            }
            return context.Fail(AssetXmlErrorKind::Parse, path, "boolean value is invalid");
        }
        else if constexpr (std::integral<TValue>)
        {
            const auto text = GetScalarText(element);
            if (!text || !ParseInteger(*text, output))
            {
                return context.Fail(AssetXmlErrorKind::Parse, path, "integer value is invalid");
            }
            return true;
        }
        else if constexpr (std::floating_point<TValue>)
        {
            const auto text = GetScalarText(element);
            if (!text || !ParseFloatingPoint(*text, output))
            {
                return context.Fail(AssetXmlErrorKind::Parse, path, "floating-point value is invalid");
            }
            return true;
        }
        else if constexpr (std::same_as<TValue, Vector3>)
        {
            return ReadVectorComponent(element, "x", output.x, context, path + ".x")
                && ReadVectorComponent(element, "y", output.y, context, path + ".y")
                && ReadVectorComponent(element, "z", output.z, context, path + ".z");
        }
        else if constexpr (std::same_as<TValue, Vector4>)
        {
            return ReadVectorComponent(element, "x", output.x, context, path + ".x")
                && ReadVectorComponent(element, "y", output.y, context, path + ".y")
                && ReadVectorComponent(element, "z", output.z, context, path + ".z")
                && ReadVectorComponent(element, "w", output.w, context, path + ".w");
        }
        else if constexpr (ReflectedAssetEnum<TValue>)
        {
            const auto text = GetScalarText(element);
            if (!text || text->empty())
            {
                return context.Fail(AssetXmlErrorKind::Parse, path, "enum value is missing");
            }

            constexpr auto values = AssetEnumReflection<TValue>::GetValues();
            for (const auto& candidate : values)
            {
                if (EqualsAsciiInsensitive(*text, candidate.name)
                    || EqualsAsciiInsensitive(*text, candidate.display_name)
                    || EqualsAsciiInsensitive(*text, candidate.alias))
                {
                    output = candidate.value;
                    return true;
                }
            }

            using TUnderlying = std::underlying_type_t<TValue>;
            TUnderlying numeric_value{};
            if (ParseInteger(*text, numeric_value))
            {
                for (const auto& candidate : values)
                {
                    if (static_cast<TUnderlying>(candidate.value) == numeric_value)
                    {
                        output = candidate.value;
                        return true;
                    }
                }
            }
            return context.Fail(AssetXmlErrorKind::Parse, path, "enum value is not defined");
        }
        else if constexpr (OptionalTraits<TValue>::kIsOptional)
        {
            using TInner = typename OptionalTraits<TValue>::ValueType;
            if (element.GetText() == nullptr && element.FirstChildElement() == nullptr)
            {
                output.reset();
                return true;
            }

            auto value = ReadConstructedValue<TInner>(element, context, path);
            if (!value)
            {
                return false;
            }
            output.emplace(std::move(*value));
            return true;
        }
        else if constexpr (VectorTraits<TValue>::kIsVector)
        {
            using TElement = typename VectorTraits<TValue>::ValueType;
            TValue values;
            std::size_t index = 0;
            for (const tinyxml2::XMLElement* child = element.FirstChildElement();
                 child != nullptr;
                 child = child->NextSiblingElement(), ++index)
            {
                if (std::string_view{child->Name()} != "item")
                {
                    return context.Fail(AssetXmlErrorKind::Parse, path, "array contains a non-item element");
                }
                auto value = ReadConstructedValue<TElement>(
                    *child,
                    context,
                    path + "[" + std::to_string(index) + "]");
                if (!value)
                {
                    return false;
                }
                values.emplace_back(std::move(*value));
            }
            output = std::move(values);
            return true;
        }
        else if constexpr (StringMapTraits<TValue>::kIsStringMap)
        {
            using TMapped = typename StringMapTraits<TValue>::ValueType;
            TValue values;
            for (const tinyxml2::XMLElement* child = element.FirstChildElement();
                 child != nullptr;
                 child = child->NextSiblingElement())
            {
                if (std::string_view{child->Name()} != "item")
                {
                    return context.Fail(AssetXmlErrorKind::Parse, path, "map contains a non-item element");
                }
                const char* key_attribute = child->Attribute("key");
                if (key_attribute == nullptr || std::string_view{key_attribute}.empty())
                {
                    return context.Fail(AssetXmlErrorKind::Parse, path, "map item key is missing");
                }
                std::string key{key_attribute};
                if (values.contains(key))
                {
                    return context.Fail(AssetXmlErrorKind::Parse, path + "." + key, "map key is duplicated");
                }

                auto value = ReadConstructedValue<TMapped>(*child, context, path + "." + key);
                if (!value)
                {
                    return false;
                }
                values.emplace(std::move(key), std::move(*value));
            }
            output = std::move(values);
            return true;
        }
        else if constexpr (AssetRefTraits<TValue>::kIsAssetRef || std::same_as<TValue, RawAssetRef>)
        {
            auto value = ReadConstructedValue<TValue>(element, context, path);
            if (!value)
            {
                return false;
            }
            output = std::move(*value);
            return true;
        }
        else if constexpr (ReflectedAssetObject<TValue>)
        {
            return ReadObject(element, output, context, path);
        }
        else
        {
            static_assert(!sizeof(TValue), "Unsupported C++ asset field type");
        }
    }

    template<class TValue>
    [[nodiscard]] bool ValidateValue(
        const TValue& value,
        const AssetFieldOptions& options,
        AssetXmlReadContext& context,
        const std::string& path)
    {
        if constexpr (OptionalTraits<TValue>::kIsOptional)
        {
            if (options.required && !value)
            {
                return context.Fail(AssetXmlErrorKind::Validation, path, "required value is empty");
            }
            if (value)
            {
                return ValidateValue(*value, options, context, path);
            }
        }
        else if constexpr ((std::integral<TValue> || std::floating_point<TValue>)
                           && !std::same_as<TValue, bool>)
        {
            const double numeric_value = static_cast<double>(value);
            if (options.minimum && numeric_value < *options.minimum)
            {
                return context.Fail(AssetXmlErrorKind::Validation, path, "value is below its minimum");
            }
            if (options.maximum && numeric_value > *options.maximum)
            {
                return context.Fail(AssetXmlErrorKind::Validation, path, "value is above its maximum");
            }
        }
        return true;
    }

    template<class TObject, class TField>
    [[nodiscard]] bool ReadField(
        const tinyxml2::XMLElement& element,
        TObject& output,
        const TField& field,
        AssetXmlReadContext& context,
        const std::string& object_path)
    {
        const tinyxml2::XMLElement* field_element = nullptr;
        for (const tinyxml2::XMLElement* child = element.FirstChildElement();
             child != nullptr;
             child = child->NextSiblingElement())
        {
            if (std::string_view{child->Name()} != field.GetName())
            {
                continue;
            }
            if (field_element != nullptr)
            {
                return context.Fail(
                    AssetXmlErrorKind::Parse,
                    object_path + "." + std::string{field.GetName()},
                    "field is duplicated");
            }
            field_element = child;
        }

        const std::string field_path = object_path + "." + std::string{field.GetName()};
        if (field_element == nullptr)
        {
            if (field.GetOptions().required)
            {
                return context.Fail(AssetXmlErrorKind::Validation, field_path, "required field is missing");
            }
            return true;
        }

        auto& value = output.*TField::kMember;
        return ReadValue(*field_element, value, context, field_path)
            && ValidateValue(value, field.GetOptions(), context, field_path);
    }

    template<ReflectedAssetObject TObject>
    [[nodiscard]] bool ReadObject(
        const tinyxml2::XMLElement& element,
        TObject& output,
        AssetXmlReadContext& context,
        const std::string& path)
    {
        constexpr auto schema = AssetReflection<TObject>::GetSchema();

        for (const tinyxml2::XMLElement* child = element.FirstChildElement();
             child != nullptr;
             child = child->NextSiblingElement())
        {
            bool known = false;
            std::apply(
                [&](const auto&... fields)
                {
                    ((known = known || std::string_view{child->Name()} == fields.GetName()), ...);
                },
                schema.GetFields());
            if (!known)
            {
                return context.Fail(
                    AssetXmlErrorKind::Parse,
                    path + "." + child->Name(),
                    "field is not declared by the C++ asset description");
            }
        }

        bool success = true;
        std::apply(
            [&](const auto&... fields)
            {
                ((success = success && ReadField(element, output, fields, context, path)), ...);
            },
            schema.GetFields());
        return success;
    }

    template<AssetDescription TAsset>
        requires ReflectedAssetObject<TAsset>
    [[nodiscard]] AssetLoadError LoadXmlAssetFile(
        const std::string& file_path,
        void* output_asset)
    {
        if (output_asset == nullptr)
        {
            return AssetLoadError::AssetFieldParseFailed;
        }

        tinyxml2::XMLDocument document;
        const tinyxml2::XMLError load_error = document.LoadFile(file_path.c_str());
        switch (load_error)
        {
        case tinyxml2::XML_SUCCESS:
            break;
        case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
        case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
        case tinyxml2::XML_ERROR_FILE_READ_ERROR:
            return AssetLoadError::FileReadFailed;
        case tinyxml2::XML_ERROR_EMPTY_DOCUMENT:
            return AssetLoadError::XmlRootMissing;
        default:
            return AssetLoadError::XmlParseFailed;
        }

        const tinyxml2::XMLElement* root = document.RootElement();
        if (root == nullptr)
        {
            return AssetLoadError::XmlRootMissing;
        }
        if (std::string_view{root->Name()} != "asset")
        {
            LOG_ERROR("Asset '{0}' must use an <asset> root element", file_path);
            return AssetLoadError::AssetMetadataInvalid;
        }

        const char* type_id = root->Attribute("type");
        if (type_id == nullptr || std::string_view{type_id} != TAsset::kTypeId)
        {
            LOG_ERROR("Asset '{0}' has a missing or incorrect type id; expected '{1}'", file_path, TAsset::kTypeId);
            return AssetLoadError::AssetMetadataInvalid;
        }

        const char* version_text = root->Attribute("version");
        std::uint32_t version{};
        if (version_text == nullptr || !ParseInteger(Trim(version_text), version))
        {
            LOG_ERROR("Asset '{0}' has an invalid schema version", file_path);
            return AssetLoadError::AssetMetadataInvalid;
        }
        if (version != TAsset::kSchemaVersion)
        {
            LOG_ERROR(
                "Asset '{0}' uses unsupported schema version {1}; expected {2}",
                file_path,
                version,
                TAsset::kSchemaVersion);
            return AssetLoadError::AssetVersionUnsupported;
        }

        AssetXmlReadContext context;
        auto& typed_output = *static_cast<TAsset*>(output_asset);
        if (!ReadObject(*root, typed_output, context, std::string{TAsset::kTypeId}))
        {
            LOG_ERROR(
                "Failed to read asset '{0}' at '{1}': {2}",
                file_path,
                context.path,
                context.message);
            return context.kind == AssetXmlErrorKind::Validation
                ? AssetLoadError::AssetValidationFailed
                : AssetLoadError::AssetFieldParseFailed;
        }
        return AssetLoadError::None;
    }
}

#endif // DOLAS_ASSET_XML_READER_H
