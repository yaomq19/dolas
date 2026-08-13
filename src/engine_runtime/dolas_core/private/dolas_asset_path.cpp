#include "dolas_asset_path.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace Dolas
{
    namespace
    {
        // Mount prefixes are part of an asset's canonical logical identity.
        constexpr std::string_view k_engine_prefix{"_engine/"};
        constexpr std::string_view k_project_prefix{"_project/"};

        // These characters are unsafe or non-portable in file names.
        constexpr std::string_view k_invalid_path_characters{"<>:\"|?*"};

        // Validates one path segment without consulting the filesystem.
        [[nodiscard]] bool IsPortablePathSegment(std::string_view segment) noexcept
        {
            return std::ranges::none_of(segment, [](const char character)
            {
                const auto unsigned_character = static_cast<unsigned char>(character);
                return unsigned_character < 0x20 || k_invalid_path_characters.find(character) != std::string_view::npos;
            });
        }
    }

    // Private construction preserves the invariants enforced by Parse().
    AssetPath::AssetPath(AssetMount mount, std::string relative_path, std::string canonical_path)
        : m_mount{mount}
        , m_relative_path{std::move(relative_path)}
        , m_canonical_path{std::move(canonical_path)}
    {
    }

    std::optional<AssetPath> AssetPath::Parse(std::string_view value)
    {
        if (value.empty() || value.find('\0') != std::string_view::npos)
        {
            return std::nullopt;
        }

        // Accept either slash style and normalize separators to forward slashes.
        std::string normalized{value};
        std::ranges::replace(normalized, '\\', '/');

        AssetMount mount{};
        std::size_t prefix_size{};
        std::string_view canonical_prefix;

        // Require an explicit mount so path resolution is never ambiguous.
        if (normalized.starts_with(k_engine_prefix))
        {
            mount = AssetMount::Engine;
            prefix_size = k_engine_prefix.size();
            canonical_prefix = k_engine_prefix;
        }
        else if (normalized.starts_with(k_project_prefix))
        {
            mount = AssetMount::Project;
            prefix_size = k_project_prefix.size();
            canonical_prefix = k_project_prefix;
        }
        else
        {
            return std::nullopt;
        }

        const std::string_view input_relative_path{normalized.data() + prefix_size, normalized.size() - prefix_size};
        std::string relative_path;

        // Collapse empty and current-directory segments while rejecting traversal.
        std::size_t segment_begin{};
        while (segment_begin <= input_relative_path.size())
        {
            const std::size_t separator = input_relative_path.find('/', segment_begin);
            const std::size_t segment_end = separator == std::string_view::npos
                ? input_relative_path.size()
                : separator;
            const std::string_view segment = input_relative_path.substr(segment_begin, segment_end - segment_begin);

            if (segment == ".." || (!segment.empty() && segment != "." && !IsPortablePathSegment(segment)))
            {
                return std::nullopt;
            }

            if (!segment.empty() && segment != ".")
            {
                if (!relative_path.empty())
                {
                    relative_path.push_back('/');
                }
                relative_path.append(segment);
            }

            if (separator == std::string_view::npos)
            {
                break;
            }
            segment_begin = separator + 1;
        }

        if (relative_path.empty())
        {
            return std::nullopt;
        }

        // Preserve the mount prefix in the stable form used for identity and hashing.
        std::string canonical_path{canonical_prefix};
        canonical_path.append(relative_path);
        return AssetPath{mount, std::move(relative_path), std::move(canonical_path)};
    }

    AssetMount AssetPath::GetMount() const noexcept
    {
        return m_mount;
    }

    std::string_view AssetPath::GetRelativePath() const noexcept
    {
        return m_relative_path;
    }

    const std::string& AssetPath::GetCanonicalPath() const noexcept
    {
        return m_canonical_path;
    }

    std::size_t AssetPathHash::operator()(const AssetPath& path) const noexcept
    {
        // Equivalent input spellings share the same canonical hash key.
        return std::hash<std::string_view>{}(path.GetCanonicalPath());
    }
}
