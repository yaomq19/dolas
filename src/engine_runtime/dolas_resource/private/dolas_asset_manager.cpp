#include "dolas_asset_manager.h"

#include "asset_types/camera_asset.h"
#include "asset_types/entity_asset.h"
#include "asset_types/material_asset.h"
#include "asset_types/mesh_asset.h"
#include "asset_types/scene_asset.h"
#include "dolas_asset_xml_reader.h"

namespace Dolas
{
    AssetManager::AssetManager() = default;

    AssetManager::~AssetManager()
    {
        Clear();
    }

    Bool AssetManager::Initialize()
    {
        Clear();
        m_asset_loaders.clear();
        m_asset_loaders.emplace(
            CameraAssetDesc::kTypeId,
            &Detail::LoadXmlAssetFile<CameraAssetDesc>);
        m_asset_loaders.emplace(
            EntityAssetDesc::kTypeId,
            &Detail::LoadXmlAssetFile<EntityAssetDesc>);
        m_asset_loaders.emplace(
            MaterialAssetDesc::kTypeId,
            &Detail::LoadXmlAssetFile<MaterialAssetDesc>);
        m_asset_loaders.emplace(
            MeshAssetDesc::kTypeId,
            &Detail::LoadXmlAssetFile<MeshAssetDesc>);
        m_asset_loaders.emplace(
            SceneAssetDesc::kTypeId,
            &Detail::LoadXmlAssetFile<SceneAssetDesc>);
        return true;
    }

    Bool AssetManager::Clear()
    {
        for (auto& [asset_type, cache] : m_asset_caches)
        {
            (void)asset_type;
            if (cache)
            {
                cache->Clear();
            }
        }
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
