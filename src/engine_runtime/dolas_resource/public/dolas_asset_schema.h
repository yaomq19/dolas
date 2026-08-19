#ifndef DOLAS_ASSET_SCHEMA_H
#define DOLAS_ASSET_SCHEMA_H

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Dolas
{
    // Optional authoring and validation metadata shared by loaders and editors.
    struct AssetFieldOptions
    {
        bool required = false;
        std::optional<double> minimum;
        std::optional<double> maximum;
        std::string_view display_name;
        std::string_view tooltip;
    };

    namespace Detail
    {
        template<class T>
        struct MemberPointerTraits;

        template<class TObject, class TValue>
        struct MemberPointerTraits<TValue TObject::*>
        {
            using ObjectType = TObject;
            using ValueType = TValue;
        };

        template<class... TFields>
        consteval bool HaveUniqueFieldIds()
        {
            constexpr std::array ids{TFields::kId...};
            for (std::size_t left = 0; left < ids.size(); ++left)
            {
                for (std::size_t right = left + 1; right < ids.size(); ++right)
                {
                    if (ids[left] == ids[right])
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        template<class... TFields>
        consteval bool HaveUniqueFieldNames(const TFields&... fields)
        {
            const std::array names{fields.GetName()...};
            for (std::size_t left = 0; left < names.size(); ++left)
            {
                for (std::size_t right = left + 1; right < names.size(); ++right)
                {
                    if (names[left] == names[right])
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    }

    // Describes one serialized field through a type-safe pointer-to-member.
    template<std::uint32_t TId, auto TMember>
    class AssetField final
    {
        static_assert(std::is_member_object_pointer_v<decltype(TMember)>);

    public:
        using ObjectType = typename Detail::MemberPointerTraits<decltype(TMember)>::ObjectType;
        using ValueType = typename Detail::MemberPointerTraits<decltype(TMember)>::ValueType;

        static constexpr std::uint32_t kId = TId;
        static constexpr auto kMember = TMember;

        constexpr AssetField(std::string_view name, AssetFieldOptions options) noexcept
            : m_name{name}
            , m_options{options}
        {
        }

        [[nodiscard]] constexpr std::string_view GetName() const noexcept
        {
            return m_name;
        }

        [[nodiscard]] constexpr const AssetFieldOptions& GetOptions() const noexcept
        {
            return m_options;
        }

    private:
        std::string_view m_name;
        AssetFieldOptions m_options;
    };

    template<std::uint32_t TId, auto TMember>
    [[nodiscard]] constexpr auto MakeAssetField(
        std::string_view name,
        AssetFieldOptions options = {}) noexcept
    {
        return AssetField<TId, TMember>{name, options};
    }

    // Compile-time field collection for one C++ asset description type.
    template<class TObject, class... TFields>
    class AssetSchema final
    {
        static_assert((std::same_as<TObject, typename TFields::ObjectType> && ...));
        static_assert(Detail::HaveUniqueFieldIds<TFields...>());

    public:
        constexpr explicit AssetSchema(TFields... fields) noexcept
            : m_fields{std::move(fields)...}
        {
        }

        [[nodiscard]] constexpr const std::tuple<TFields...>& GetFields() const noexcept
        {
            return m_fields;
        }

    private:
        std::tuple<TFields...> m_fields;
    };

    template<class TObject, class... TFields>
    [[nodiscard]] consteval auto MakeAssetSchema(TFields... fields)
    {
        if (!Detail::HaveUniqueFieldNames(fields...))
        {
            throw "Asset field names must be unique";
        }
        return AssetSchema<TObject, TFields...>{std::move(fields)...};
    }

    // Specialized beside each authoritative C++ asset description.
    template<class TObject>
    struct AssetReflection;

    template<class TEnum>
    struct AssetEnumReflection;

    template<class TEnum>
    struct AssetEnumValue
    {
        TEnum value;
        std::string_view name;
        std::string_view display_name;
        std::string_view alias;
    };

    template<class TObject>
    concept ReflectedAssetObject = requires
    {
        AssetReflection<TObject>::GetSchema();
    };

    template<class TEnum>
    concept ReflectedAssetEnum = std::is_enum_v<TEnum> && requires
    {
        AssetEnumReflection<TEnum>::GetValues();
    };

    // A root asset is a value type with stable serialized identity.
    template<class TAsset>
    concept AssetDescription = std::default_initializable<TAsset>
        && std::move_constructible<TAsset>
        && requires
        {
            { TAsset::kTypeId } -> std::convertible_to<std::string_view>;
            { TAsset::kFileSuffix } -> std::convertible_to<std::string_view>;
            { TAsset::kSchemaVersion } -> std::convertible_to<std::uint32_t>;
        };
}

#endif // DOLAS_ASSET_SCHEMA_H
