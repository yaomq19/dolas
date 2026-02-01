#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "core/dolas_engine.h"
#include "manager/dolas_material_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_entity.h"
#include "manager/dolas_asset_manager.h" // GetRsdAsset<>
#include "manager/dolas_render_primitive_manager.h"
#include "rsd/entity.h"
#include "rsd/mesh.h"
namespace Dolas
{
    RenderEntityManager::RenderEntityManager()
    {

    }

    RenderEntityManager::~RenderEntityManager()
    {

    }
    
    bool RenderEntityManager::Initialize()
    {
        return true;
    }
    
    bool RenderEntityManager::Clear()
    {
        for (auto it = m_render_entities.begin(); it != m_render_entities.end(); ++it)
        {
            RenderEntity* render_entity = it->second;
            if (render_entity)
            {
                render_entity->Clear();
                DOLAS_DELETE(render_entity);
            }
        }
        m_render_entities.clear();
        return true;
    }
#pragma optimize("", off)
    RenderEntityID RenderEntityManager::CreateRenderEntityFromFile(const std::string& entity_file_name,
                                                                   const Vector3& position,
                                                                   const Quaternion& rotation,
                                                                   const Vector3& scale)
    {
        RenderEntityID result_id = RENDER_ENTITY_ID_EMPTY;
		RenderPrimitiveID render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
		MaterialID material_id = MATERIAL_ID_EMPTY;

        std::string render_entity_file_path = PathUtils::GetContentDir() + entity_file_name;

        EntityRSD* entity_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<EntityRSD>(entity_file_name);
        if (entity_rsd == nullptr)
            return result_id;

        // 创建 RenderEntity
        RenderEntity* render_entity = DOLAS_NEW(RenderEntity);
        render_entity->m_file_id = HashConverter::StringHash(render_entity_file_path);
        render_entity->m_pose.m_postion = position;
        render_entity->m_pose.m_rotation = rotation;
        render_entity->m_pose.m_scale = scale;

        // 遍历所有 Mesh 并加载
        for (const auto& mesh_file : entity_rsd->meshes)
        {
            if (mesh_file.empty()) continue;

            // 加载 Mesh 数据
            RenderPrimitiveID primitive_id = g_dolas_engine.m_render_primitive_manager->CreateRenderPrimitiveFromMeshFile(mesh_file);
            if (primitive_id == RENDER_PRIMITIVE_ID_EMPTY) continue;

            // 从 Mesh 资产中获取材质路径
            MeshRSD* mesh_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<MeshRSD>(mesh_file);
            MaterialID material_id = MATERIAL_ID_EMPTY;
            if (mesh_rsd && !mesh_rsd->material.empty())
            {
                material_id = g_dolas_engine.m_material_manager->CreateMaterial(mesh_rsd->material);
            }

            render_entity->AddComponent(primitive_id, material_id);
        }

        m_render_entities[render_entity->m_file_id] = render_entity;
        result_id = render_entity->m_file_id;
        
        return result_id;
    }

    RenderEntity* RenderEntityManager::GetRenderEntityByID(RenderEntityID render_entity_id)
    {
        auto it = m_render_entities.find(render_entity_id);
        if (it != m_render_entities.end())
        {
            return it->second;
        }
        return nullptr;
    }

    RenderEntity* RenderEntityManager::GetRenderEntityByFileName(const std::string& render_entity_file_name)
    {
        std::string render_entity_file_path = PathUtils::GetContentDir() + render_entity_file_name;
        RenderEntityID render_entity_id = HashConverter::StringHash(render_entity_file_path);
        return GetRenderEntityByID(render_entity_id);
    }
}