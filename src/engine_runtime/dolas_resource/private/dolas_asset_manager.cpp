#include "dolas_asset_manager.h"

#include "asset_types/camera_asset.h"
#include "asset_types/entity_asset.h"
#include "asset_types/material_asset.h"
#include "asset_types/mesh_asset.h"
#include "asset_types/scene_asset.h"
#include "dolas_asset_json_reader.h"

namespace
{
    [[nodiscard]] Dolas::AssetLoadError LoadCameraAssetFile(
        const std::string& file_path,
        void* output_asset)
    {
        using namespace Dolas;

        const AssetLoadError error = Detail::LoadJsonAssetFile<CameraAssetDesc>(file_path, output_asset);
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
            &Detail::LoadJsonAssetFile<EntityAssetDesc>);
        m_asset_loaders.emplace(
            MaterialAssetDesc::kTypeId,
            &Detail::LoadJsonAssetFile<MaterialAssetDesc>);
        m_asset_loaders.emplace(
            MeshAssetDesc::kTypeId,
            &Detail::LoadJsonAssetFile<MeshAssetDesc>);
        m_asset_loaders.emplace(
            SceneAssetDesc::kTypeId,
            &Detail::LoadJsonAssetFile<SceneAssetDesc>);
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
