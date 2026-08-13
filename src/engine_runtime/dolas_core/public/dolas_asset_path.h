#ifndef DOLAS_ASSET_PATH_H
#define DOLAS_ASSET_PATH_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace Dolas
{
    enum class AssetMount
    {
        Engine,
        Project
    };

    class AssetPath
    {
    public:
        [[nodiscard]] static std::optional<AssetPath> Parse(std::string_view value);

        [[nodiscard]] AssetMount GetMount() const noexcept;
        [[nodiscard]] std::string_view GetRelativePath() const noexcept;
        [[nodiscard]] const std::string& GetCanonicalPath() const noexcept;

        friend bool operator==(const AssetPath&, const AssetPath&) = default;

    private:
        AssetPath(AssetMount mount, std::string relative_path, std::string canonical_path);

        AssetMount m_mount;
        std::string m_relative_path;
        std::string m_canonical_path;
    };

    struct AssetPathHash
    {
        [[nodiscard]] std::size_t operator()(const AssetPath& path) const noexcept;
    };
}

#endif // DOLAS_ASSET_PATH_H
