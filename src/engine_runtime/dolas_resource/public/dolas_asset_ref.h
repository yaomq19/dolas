#ifndef DOLAS_ASSET_REF_H
#define DOLAS_ASSET_REF_H

#include <utility>

#include "dolas_asset_path.h"

namespace Dolas
{
    // References another structured asset and preserves its target type in C++.
    template<class TAsset>
    class AssetRef final
    {
    public:
        explicit AssetRef(AssetPath path)
            : m_path{std::move(path)}
        {
        }

        [[nodiscard]] const AssetPath& GetPath() const noexcept
        {
            return m_path;
        }

        friend bool operator==(const AssetRef&, const AssetRef&) = default;

    private:
        AssetPath m_path;
    };

    // References a non-structured source asset such as a shader or texture.
    class RawAssetRef final
    {
    public:
        explicit RawAssetRef(AssetPath path)
            : m_path{std::move(path)}
        {
        }

        [[nodiscard]] const AssetPath& GetPath() const noexcept
        {
            return m_path;
        }

        friend bool operator==(const RawAssetRef&, const RawAssetRef&) = default;

    private:
        AssetPath m_path;
    };
}

#endif // DOLAS_ASSET_REF_H
