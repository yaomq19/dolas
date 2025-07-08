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

    RenderEntity* RenderEntityManager::GetOrCreateRenderEntity(const std::string& file_name)
    {
        if (m_render_entities.find(file_name) == m_render_entities.end())
        {
            RenderEntity* render_entity = CreateRenderEntity(file_name);
            if (render_entity != nullptr)
            {
                m_render_entities[file_name] = render_entity;
            }
            return render_entity;
        }
        return m_render_entities[file_name];
    }

    RenderEntity* RenderEntityManager::CreateRenderEntity(const std::string& file_name)
    {
        json json_data;
        std::string entity_path = PathUtils::GetEntityDir() + file_name;

        std::ifstream file(entity_path);
        if (!file.is_open())
        {
            std::cerr << "RenderEntity::Load: file is not found in " << entity_path << std::endl;
            return nullptr;
        }

        file >> json_data;
        file.close();

        RenderEntity* render_entity = DOLAS_NEW(RenderEntity);
        render_entity->m_file_path = entity_path;

        MeshManager* mesh_manager = g_dolas_engine.m_mesh_manager;
        DOLAS_RETURN_NULL_IF_NULL(mesh_manager);
        MaterialManager* material_manager = g_dolas_engine.m_material_manager;
        DOLAS_RETURN_NULL_IF_NULL(material_manager);

        if (json_data.contains("mesh"))
        {
            std::string mesh_file_name = json_data["mesh"];
            render_entity->m_mesh = mesh_manager->GetOrCreateMesh(mesh_file_name);
            DOLAS_RETURN_NULL_IF_NULL(render_entity->m_mesh);
        }

        if (json_data.contains("material"))
        {
            std::string material_file_name = json_data["material"];
            render_entity->m_material = material_manager->GetOrCreateMaterial(material_file_name);
            DOLAS_RETURN_NULL_IF_NULL(render_entity->m_material);
        }

        return render_entity;
    }
}