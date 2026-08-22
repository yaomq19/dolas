#ifndef DOLAS_ASSET_REF_H
#define DOLAS_ASSET_REF_H

#include <utility>

#include "dolas_asset_path.h"

namespace Dolas
{
    // Tag for references to non-structured source assets such as shaders or textures.
    struct RawAssetTag
    {
    };

    // References another asset and preserves its target type in C++.
    // TAsset is a phantom parameter: it is never instantiated, it only records the target.
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

    using RawAssetRef = AssetRef<RawAssetTag>;
}

#endif // DOLAS_ASSET_REF_H
