#include "manager/dolas_render_object_manager.h"
#include "render/dolas_render_object.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_entity_manager.h"
#include "base/dolas_base.h"
#include "core/dolas_rhi.h"
#include <iostream>

namespace Dolas
{
    RenderObjectManager::RenderObjectManager()
        : m_next_object_id(1) // Start from 1, 0 is reserved for EMPTY
    {
    }

    RenderObjectManager::~RenderObjectManager()
    {
    }

    bool RenderObjectManager::Initialize()
    {
        return true;
    }

    bool RenderObjectManager::Clear()
    {
        for (auto& pair : m_render_objects)
        {
            RenderObject* render_object = pair.second;
            if (render_object)
            {
            }
        }
        m_render_objects.clear();
        m_name_to_id_map.clear();
        m_next_object_id = 1;
        return true;
    }

    void RenderObjectManager::Update(Float delta_time)
    {

    }

    RenderObjectID RenderObjectManager::CreateRenderObject()
    {
        return CreateRenderObject("RenderObject_" + std::to_string(m_next_object_id));
    }

    RenderObjectID RenderObjectManager::CreateRenderObject(const std::string& name)
    {
        RenderObjectID object_id = GenerateNewObjectID();
        
        RenderObject* render_object = DOLAS_NEW(RenderObject);
        render_object->m_object_id = object_id;

        
        m_render_objects[object_id] = render_object;
        m_name_to_id_map[name] = object_id;
        
        return object_id;
    }

    RenderObjectID RenderObjectManager::CreateRenderObjectFromEntity(const std::string& entity_file_name)
    {
        return CreateRenderObjectFromEntity(entity_file_name, Vector3(0.0f, 0.0f, 0.0f));
    }

    RenderObjectID RenderObjectManager::CreateRenderObjectFromEntity(const std::string& entity_file_name, const Vector3& position)
    {
        return CreateRenderObjectFromEntity(entity_file_name, position, Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
    }

    RenderObjectID RenderObjectManager::CreateRenderObjectFromEntity(const std::string& entity_file_name, 
                                                                   const Vector3& position, 
                                                                   const Vector3& rotation, 
                                                                   const Vector3& scale)
    {
        // Create or get the render entity
        RenderEntityID entity_id = g_dolas_engine.m_render_entity_manager->CreateRenderEntityFromFile(entity_file_name);
        if (entity_id == RENDER_ENTITY_ID_EMPTY)
        {
            std::cerr << "Failed to create render entity from file: " << entity_file_name << std::endl;
            return RENDER_OBJECT_ID_EMPTY;
        }

        // Create the render object
        std::string object_name = "Object_" + entity_file_name + "_" + std::to_string(m_next_object_id);
        RenderObjectID object_id = CreateRenderObject(object_name);
        if (object_id == RENDER_OBJECT_ID_EMPTY)
        {
            return RENDER_OBJECT_ID_EMPTY;
        }

        // Configure the render object
        RenderObject* render_object = GetRenderObjectByID(object_id);

        return object_id;
    }

    bool RenderObjectManager::DestroyRenderObject(RenderObjectID object_id)
    {
        auto it = m_render_objects.find(object_id);
        if (it == m_render_objects.end())
        {
            return false;
        }

        RenderObject* render_object = it->second;
        if (render_object)
        {
            // Remove from name map

            // Clean up and delete
            DOLAS_DELETE(render_object);
        }

        m_render_objects.erase(it);
        return true;
    }

    RenderObject* RenderObjectManager::GetRenderObjectByID(RenderObjectID object_id)
    {
        auto it = m_render_objects.find(object_id);
        if (it != m_render_objects.end())
        {
            return it->second;
        }
        return nullptr;
    }

    RenderObject* RenderObjectManager::GetRenderObjectByName(const std::string& name)
    {
        auto it = m_name_to_id_map.find(name);
        if (it != m_name_to_id_map.end())
        {
            return GetRenderObjectByID(it->second);
        }
        return nullptr;
    }

    void RenderObjectManager::DrawAllObjects(DolasRHI* rhi)
    {
        for (auto& pair : m_render_objects)
        {
            RenderObject* render_object = pair.second;

        }
    }

    void RenderObjectManager::SetAllObjectsVisible(bool visible)
    {
        for (auto& pair : m_render_objects)
        {
            RenderObject* render_object = pair.second;
            if (render_object)
            {
            }
        }
    }

    void RenderObjectManager::ClearAllObjects()
    {
        Clear();
    }

    UInt RenderObjectManager::GetObjectCount() const
    {
        return static_cast<UInt>(m_render_objects.size());
    }

    std::vector<RenderObjectID> RenderObjectManager::GetAllObjectIDs() const
    {
        std::vector<RenderObjectID> object_ids;
        object_ids.reserve(m_render_objects.size());
        
        for (const auto& pair : m_render_objects)
        {
            object_ids.push_back(pair.first);
        }
        
        return object_ids;
    }

    RenderObjectID RenderObjectManager::GenerateNewObjectID()
    {
        return m_next_object_id++;
    }

} // namespace Dolas
