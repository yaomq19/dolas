#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "core/dolas_engine.h"
#include "manager/dolas_material_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_entity.h"
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_render_primitive_manager.h"
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
    RenderEntityID RenderEntityManager::CreateRenderEntityFromFile(const SceneEntity& scene_entity)
    {
        RenderEntityID result_id = RENDER_ENTITY_ID_EMPTY;
		RenderPrimitiveID render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
		MaterialID material_id = MATERIAL_ID_EMPTY;

        std::string render_entity_file_path = PathUtils::GetEntityDir() + scene_entity.entity_file;

        EntityRSD* entity_rsd = g_dolas_engine.m_asset_manager->GetRsdAsset<EntityRSD>(PathUtils::GetEntityDir() + scene_entity.entity_file);
        if (entity_rsd == nullptr)
            return result_id;

        const std::string& type_name = entity_rsd->type;
        if (!type_name.empty())
        {
            if (type_name == "base_geometry")
            {
                const std::string& base_geometry_name = entity_rsd->base_geometry;
                if (base_geometry_name == "cube")
                {
                    render_primitive_id = g_dolas_engine.m_render_primitive_manager->GetGeometryRenderPrimitiveID(BaseGeometryType_CUBE);
                }
                else if (base_geometry_name == "sphere")
                {
                    render_primitive_id = g_dolas_engine.m_render_primitive_manager->GetGeometryRenderPrimitiveID(BaseGeometryType_SPHERE);
                }
                else
                {
                    return result_id;
                }
            }
            else if (type_name == "mesh")
            {
                RenderPrimitiveManager* primitive_manager = g_dolas_engine.m_render_primitive_manager;
                if (primitive_manager == nullptr)
                {
                    return result_id;
                }
                std::string mesh_file_name = entity_rsd->mesh;
                if (mesh_file_name.empty())
                    return result_id;

                render_primitive_id = primitive_manager->CreateRenderPrimitiveFromMeshFile(mesh_file_name);
                if (render_primitive_id == RENDER_PRIMITIVE_ID_EMPTY)
                {
                    return result_id;
                }
            }
			else
			{
				return result_id;
			}
        }
		else
		{
			return result_id;
		}

        {
            const std::string material_file_name = entity_rsd->material;
            if (material_file_name.empty())
                return result_id;

            MaterialManager* material_manager = g_dolas_engine.m_material_manager;
            if (material_manager == nullptr)
            {
                return result_id;
            }

            material_id = material_manager->CreateMaterial(material_file_name);
            if (material_id == MATERIAL_ID_EMPTY)
            {
                return result_id;
            }
        }

        // 成功路径：创建 RenderEntity 并返回有效 ID
        {
            RenderEntity* render_entity = DOLAS_NEW(RenderEntity);
            render_entity->m_file_id = HashConverter::StringHash(render_entity_file_path);
            render_entity->m_render_primitive_id = render_primitive_id;
            render_entity->m_material_id = material_id;
            render_entity->m_pose.m_postion = scene_entity.position;
            render_entity->m_pose.m_rotation = scene_entity.rotation;
            render_entity->m_pose.m_scale = scene_entity.scale;

            m_render_entities[render_entity->m_file_id] = render_entity;
            result_id = render_entity->m_file_id;
        }
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
        std::string render_entity_file_path = PathUtils::GetEntityDir() + render_entity_file_name;
        RenderEntityID render_entity_id = HashConverter::StringHash(render_entity_file_path);
        return GetRenderEntityByID(render_entity_id);
    }
}