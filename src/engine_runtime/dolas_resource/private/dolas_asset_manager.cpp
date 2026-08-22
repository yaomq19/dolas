#include "dolas_asset_manager.h"

#include <cstdint>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <rfl/json.hpp>

#include "asset_types/camera_asset.h"
#include "asset_types/entity_asset.h"
#include "asset_types/material_asset.h"
#include "asset_types/mesh_asset.h"
#include "asset_types/scene_asset.h"
#include "dolas_asset_load_result.h"
#include "dolas_asset_ref.h"
#include "dolas_asset_schema.h"
#include "dolas_log_system_manager.h"
#include "dolas_math.h"

namespace Dolas
{
    // Custom reflectors cannot report failures, so invalid reference paths throw;
    // reflect-cpp converts the exception into a parse error for LoadJsonAssetFile.
    [[nodiscard]] AssetPath ParseAssetRefPath(const std::string& value)
    {
        auto asset_path = AssetPath::Parse(value);
        if (!asset_path)
        {
            throw std::runtime_error{"invalid asset reference path: " + value};
        }
        return *asset_path;
    }

    template<class TAsset>
    struct JsonAssetFile
    {
        std::optional<std::string> type;
        std::optional<std::uint32_t> version;
        TAsset data{};
    };

    template<AssetDescription TAsset>
    [[nodiscard]] AssetLoadError LoadJsonAssetFile(
        const std::string& file_path,
        void* output_asset)
    {
        if (output_asset == nullptr)
        {
            return AssetLoadError::AssetFieldParseFailed;
        }

        std::ifstream input{file_path};
        if (!input)
        {
            return AssetLoadError::FileReadFailed;
        }

        auto result = rfl::json::read<
            JsonAssetFile<TAsset>,
            rfl::NoExtraFields,
            rfl::DefaultIfMissing>(input);
        if (!result)
        {
            LOG_ERROR("Failed to parse JSON asset '{0}': {1}", file_path, result.error().what());
            return AssetLoadError::JsonParseFailed;
        }

        auto& file = result.value();
        if (!file.type || *file.type != TAsset::kTypeId || !file.version)
        {
            LOG_ERROR("Asset '{0}' has missing or incorrect metadata", file_path);
            return AssetLoadError::AssetMetadataInvalid;
        }
        if (*file.version != TAsset::kSchemaVersion)
        {
            LOG_ERROR(
                "Asset '{0}' uses unsupported schema version {1}; expected {2}",
                file_path,
                *file.version,
                TAsset::kSchemaVersion);
            return AssetLoadError::AssetVersionUnsupported;
        }

        *static_cast<TAsset*>(output_asset) = std::move(file.data);
        return AssetLoadError::None;
    }
}

namespace rfl
{
    template<>
    struct Reflector<Dolas::Vector3>
    {
        struct ReflType
        {
            Dolas::Float x;
            Dolas::Float y;
            Dolas::Float z;
        };

        [[nodiscard]] static Dolas::Vector3 to(const ReflType& value) noexcept
        {
            return {value.x, value.y, value.z};
        }

        [[nodiscard]] static ReflType from(const Dolas::Vector3& value) noexcept
        {
            return {value.x, value.y, value.z};
        }
    };

    template<>
    struct Reflector<Dolas::Vector4>
    {
        struct ReflType
        {
            Dolas::Float x;
            Dolas::Float y;
            Dolas::Float z;
            Dolas::Float w;
        };

        [[nodiscard]] static Dolas::Vector4 to(const ReflType& value) noexcept
        {
            return {value.x, value.y, value.z, value.w};
        }

        [[nodiscard]] static ReflType from(const Dolas::Vector4& value) noexcept
        {
            return {value.x, value.y, value.z, value.w};
        }
    };

    // Asset references serialize as their canonical logical path string.
    template<Dolas::AssetDescription TAsset>
    struct Reflector<Dolas::AssetRef<TAsset>>
    {
        using ReflType = std::string;

        [[nodiscard]] static Dolas::AssetRef<TAsset> to(const ReflType& value)
        {
            return Dolas::AssetRef<TAsset>{Dolas::ParseAssetRefPath(value)};
        }

        [[nodiscard]] static ReflType from(const Dolas::AssetRef<TAsset>& value)
        {
            return std::string{value.GetPath().GetCanonicalPath()};
        }
    };

    template<>
    struct Reflector<Dolas::RawAssetRef>
    {
        using ReflType = std::string;

        [[nodiscard]] static Dolas::RawAssetRef to(const ReflType& value)
        {
            return Dolas::RawAssetRef{Dolas::ParseAssetRefPath(value)};
        }

        [[nodiscard]] static ReflType from(const Dolas::RawAssetRef& value)
        {
            return std::string{value.GetPath().GetCanonicalPath()};
        }
    };
}

namespace
{
    [[nodiscard]] Dolas::AssetLoadError LoadCameraAssetFile(
        const std::string& file_path,
        void* output_asset)
    {
        using namespace Dolas;

        const AssetLoadError error = LoadJsonAssetFile<CameraAssetDesc>(file_path, output_asset);
        if (error != AssetLoadError::None)
        {
            return error;
        }

        constexpr Float kMinimumPositiveValue{0.0001f};
        constexpr Float kMinimumFov{1.0f};
        constexpr Float kMaximumFov{179.0f};
        const auto& camera = *static_cast<const CameraAssetDesc*>(output_asset);
        if (camera.near_plane < kMinimumPositiveValue
            || camera.far_plane < kMinimumPositiveValue
            || camera.fov < kMinimumFov
            || camera.fov > kMaximumFov
            || camera.aspect_ratio < kMinimumPositiveValue
            || camera.window_width < kMinimumPositiveValue
            || camera.window_height < kMinimumPositiveValue)
        {
            LOG_ERROR("Camera asset '{0}' contains out-of-range projection values", file_path);
            return AssetLoadError::AssetValidationFailed;
        }
        return AssetLoadError::None;
    }
}

namespace Dolas
{
    Bool AssetManager::Initialize()
    {
        Clear();
        m_asset_loaders.clear();
        m_asset_loaders.emplace(
            CameraAssetDesc::kTypeId,
            &LoadCameraAssetFile);
        m_asset_loaders.emplace(
            EntityAssetDesc::kTypeId,
            &LoadJsonAssetFile<EntityAssetDesc>);
        m_asset_loaders.emplace(
            MaterialAssetDesc::kTypeId,
            &LoadJsonAssetFile<MaterialAssetDesc>);
        m_asset_loaders.emplace(
            MeshAssetDesc::kTypeId,
            &LoadJsonAssetFile<MeshAssetDesc>);
        m_asset_loaders.emplace(
            SceneAssetDesc::kTypeId,
            &LoadJsonAssetFile<SceneAssetDesc>);
        return true;
    }

    Bool AssetManager::Clear()
    {
        m_asset_caches.clear();
        return true;
    }

    AssetLoadError AssetManager::LoadAndParseAssetFile(
        std::string_view type_id,
        const std::string& file_path,
        void* output_asset) const
    {
        const auto loader = m_asset_loaders.find(std::string{type_id});
        if (loader == m_asset_loaders.end())
        {
            return AssetLoadError::AssetTypeNotRegistered;
        }
        return loader->second(file_path, output_asset);
    }
}
