#ifndef DOLAS_ASSET_PATH_H
#define DOLAS_ASSET_PATH_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace Dolas
{
    /// Selects the virtual content root for an asset.
    enum class AssetMount
    {
        Engine, ///< Content shipped with the engine.
        Project ///< Content owned by the active project.
    };

    /// A validated logical asset path with an explicit virtual mount.
    /// AssetPath does not represent a resolved filesystem path.
    class AssetPath
    {
    public:
        /// Parses and canonicalizes an `_engine/` or `_project/` logical path.
        /// Returns std::nullopt when the input is empty, ambiguous, or unsafe.
        [[nodiscard]] static std::optional<AssetPath> Parse(std::string_view value);

        /// Returns the virtual mount selected by this path.
        [[nodiscard]] AssetMount GetMount() const noexcept;

        /// Returns the normalized path relative to the mount root.
        [[nodiscard]] std::string_view GetRelativePath() const noexcept;

        /// Returns the canonical logical path, including its mount prefix.
        [[nodiscard]] const std::string& GetCanonicalPath() const noexcept;

        /// Compares normalized asset-path identities.
        friend bool operator==(const AssetPath&, const AssetPath&) = default;

    private:
        /// Constructs a path from components already validated by Parse().
        AssetPath(AssetMount mount, std::string relative_path, std::string canonical_path);

        AssetMount m_mount;             ///< Virtual content root.
        std::string m_relative_path;    ///< Normalized path within the mount.
        std::string m_canonical_path;   ///< Mount prefix plus relative path.
    };

    /// Hashes AssetPath values by their canonical logical path.
    struct AssetPathHash
    {
        /// Returns a hash suitable for unordered containers.
        [[nodiscard]] std::size_t operator()(const AssetPath& path) const noexcept;
    };
}

#endif // DOLAS_ASSET_PATH_H
