#include <iostream>
#include <fstream>
#include "base/dolas_base.h"
#include "base/dolas_paths.h"
#include "core/dolas_engine.h"
#include "manager/dolas_mesh_manager.h"
#include "manager/dolas_material_manager.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_entity.h"
#include "nlohmann/json.hpp"
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include "render/dolas_material.h"
#include "base/dolas_dx_trace.h"
#include "manager/dolas_asset_manager.h"
using json = nlohmann::json;
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

    RenderEntityID RenderEntityManager::CreateRenderEntity(const std::string& render_entity_file_name)
    {
        std::string render_entity_file_path = PathUtils::GetEntityDir() + render_entity_file_name;
        
        json json_data;
        Bool ret = g_dolas_engine.m_asset_manager->LoadJsonFile(render_entity_file_path, json_data);
        if (!ret)
        {
            return RENDER_ENTITY_ID_EMPTY;
        }

        MeshID mesh_id = MESH_ID_EMPTY;
        if (json_data.contains("mesh"))
        {
            MeshManager* mesh_manager = g_dolas_engine.m_mesh_manager;
            if (mesh_manager == nullptr)
            {
                return RENDER_ENTITY_ID_EMPTY;
            }
            std::string mesh_file_name = json_data["mesh"];
            mesh_id = mesh_manager->CreateMesh(mesh_file_name);
            if (mesh_id == MESH_ID_EMPTY)
            {
                return RENDER_ENTITY_ID_EMPTY;
            }
        }
        else
        {
            return RENDER_ENTITY_ID_EMPTY;
        }

        MaterialID material_id = MATERIAL_ID_EMPTY;
        if (json_data.contains("material"))
        {
            MaterialManager* material_manager = g_dolas_engine.m_material_manager;
            if (material_manager == nullptr)
            {
                return RENDER_ENTITY_ID_EMPTY;
            }
            std::string material_file_name = json_data["material"];
            material_id = material_manager->CreateMaterial(material_file_name);
            if (material_id == MATERIAL_ID_EMPTY)
            {
                return RENDER_ENTITY_ID_EMPTY;
            }
        }
        else
        {
            return RENDER_ENTITY_ID_EMPTY;
        }

        RenderEntity* render_entity = DOLAS_NEW(RenderEntity);
        render_entity->m_file_id = STRING_ID(render_entity_file_path);
        render_entity->m_mesh_id = mesh_id;
        render_entity->m_material_id = material_id;
        m_render_entities[render_entity->m_file_id] = render_entity;
        return render_entity->m_file_id;
    }

    RenderEntity* RenderEntityManager::GetRenderEntity(RenderEntityID render_entity_id)
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
		RenderEntityID render_entity_id = STRING_ID(render_entity_file_path);
        return GetRenderEntity(render_entity_id);
    }
}