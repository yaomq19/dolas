#ifndef DOLAS_ASSET_LOAD_RESULT_H
#define DOLAS_ASSET_LOAD_RESULT_H

namespace Dolas
{
    class AssetManager;

    // Identifies the stage at which an asset load failed.
    enum class AssetLoadError
    {
        None,
        FileSuffixMismatch,
        PathResolutionFailed,
        FileReadFailed,
        XmlParseFailed,
        XmlRootMissing,
        RsdFieldParseFailed,
    };

    // Returns a stable, human-readable name suitable for diagnostics.
    [[nodiscard]] constexpr const char* GetAssetLoadErrorName(AssetLoadError error) noexcept
    {
        switch (error)
        {
        case AssetLoadError::None:
            return "none";
        case AssetLoadError::FileSuffixMismatch:
            return "file suffix mismatch";
        case AssetLoadError::PathResolutionFailed:
            return "path resolution failed";
        case AssetLoadError::FileReadFailed:
            return "file read failed";
        case AssetLoadError::XmlParseFailed:
            return "XML parse failed";
        case AssetLoadError::XmlRootMissing:
            return "XML root missing";
        case AssetLoadError::RsdFieldParseFailed:
            return "RSD field parse failed";
        }

        return "unknown asset load error";
    }

    // Holds either a cached asset pointer or the reason loading failed.
    // The pointer remains valid until its AssetManager is cleared or destroyed.
    template<class TAsset>
    class AssetLoadResult final
    {
    public:
        [[nodiscard]] bool HasValue() const noexcept
        {
            return m_asset != nullptr;
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return HasValue();
        }

        [[nodiscard]] const TAsset* GetAsset() const noexcept
        {
            return m_asset;
        }

        [[nodiscard]] AssetLoadError GetError() const noexcept
        {
            return m_error;
        }

    private:
        friend class AssetManager;

        AssetLoadResult(const TAsset* asset, AssetLoadError error) noexcept
            : m_asset{asset}
            , m_error{error}
        {
        }

        const TAsset* m_asset = nullptr;
        AssetLoadError m_error = AssetLoadError::None;
    };
}

#endif // DOLAS_ASSET_LOAD_RESULT_H
