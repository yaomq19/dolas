#include "manager/dolas_render_model_manager.h"
#include "render/dolas_render_model.h"
#include "render/dolas_render_camera.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_object_manager.h"
#include "base/dolas_base.h"
#include "core/dolas_rhi.h"
#include <iostream>
#include <cmath>

namespace Dolas
{
    RenderModelManager::RenderModelManager()
        : m_next_model_id(1) // Start from 1, 0 is reserved for EMPTY
        , m_auto_lod_enabled(false)
    {
        // Default LOD configuration
        m_lod_configs = {
            {50.0f, 0},   // High detail within 50 units
            {100.0f, 1},  // Medium detail within 100 units
            {200.0f, 2},  // Low detail within 200 units
        };
    }

    RenderModelManager::~RenderModelManager()
    {
    }

    bool RenderModelManager::Initialize()
    {
        return true;
    }

    bool RenderModelManager::Clear()
    {
        for (auto& pair : m_render_models)
        {
            RenderModel* render_model = pair.second;
            if (render_model)
            {

            }
        }
        m_render_models.clear();
        m_name_to_id_map.clear();
        m_next_model_id = 1;
        return true;
    }

    void RenderModelManager::Update(Float delta_time)
    {
        for (auto& pair : m_render_models)
        {
            RenderModel* render_model = pair.second;
            if (render_model)
            {
            }
        }
    }

    RenderModelID RenderModelManager::CreateRenderModel()
    {
        return CreateRenderModel("RenderModel_" + std::to_string(m_next_model_id));
    }

    RenderModelID RenderModelManager::CreateRenderModel(const std::string& name)
    {
        RenderModelID model_id = GenerateNewModelID();
        
        RenderModel* render_model = DOLAS_NEW(RenderModel);
        render_model->m_model_id = model_id;

        
        m_render_models[model_id] = render_model;
        m_name_to_id_map[name] = model_id;
        
        return model_id;
    }

    RenderModelID RenderModelManager::CreateRenderModelFromObjects(const std::vector<RenderObjectID>& object_ids)
    {
        return CreateRenderModelFromObjects(object_ids, "RenderModel_" + std::to_string(m_next_model_id));
    }

    RenderModelID RenderModelManager::CreateRenderModelFromObjects(const std::vector<RenderObjectID>& object_ids, const std::string& name)
    {
        RenderModelID model_id = CreateRenderModel(name);
        if (model_id == RENDER_MODEL_ID_EMPTY)
        {
            return RENDER_MODEL_ID_EMPTY;
        }

        RenderModel* render_model = GetRenderModelByID(model_id);
        if (!render_model)
        {
            return RENDER_MODEL_ID_EMPTY;
        }

        // Add all objects to the model
        for (RenderObjectID object_id : object_ids)
        {
        }

        return model_id;
    }

    bool RenderModelManager::DestroyRenderModel(RenderModelID model_id)
    {
        auto it = m_render_models.find(model_id);
        if (it == m_render_models.end())
        {
            return false;
        }

        RenderModel* render_model = it->second;
        if (render_model)
        {

            // Clean up and delete
            DOLAS_DELETE(render_model);
        }

        m_render_models.erase(it);
        return true;
    }

    RenderModel* RenderModelManager::GetRenderModelByID(RenderModelID model_id)
    {
        auto it = m_render_models.find(model_id);
        if (it != m_render_models.end())
        {
            return it->second;
        }
        return nullptr;
    }

    RenderModel* RenderModelManager::GetRenderModelByName(const std::string& name)
    {
        auto it = m_name_to_id_map.find(name);
        if (it != m_name_to_id_map.end())
        {
            return GetRenderModelByID(it->second);
        }
        return nullptr;
    }


    void RenderModelManager::ClearAllModels()
    {
        Clear();
    }

    UInt RenderModelManager::GetModelCount() const
    {
        return static_cast<UInt>(m_render_models.size());
    }

    std::vector<RenderModelID> RenderModelManager::GetAllModelIDs() const
    {
        std::vector<RenderModelID> model_ids;
        model_ids.reserve(m_render_models.size());
        
        for (const auto& pair : m_render_models)
        {
            model_ids.push_back(pair.first);
        }
        
        return model_ids;
    }

    void RenderModelManager::SetLODConfig(const std::vector<LODConfig>& lod_configs)
    {
        m_lod_configs = lod_configs;
    }

    void RenderModelManager::EnableAutoLOD(bool enable)
    {
        m_auto_lod_enabled = enable;
    }

    RenderModelID RenderModelManager::GenerateNewModelID()
    {
        return m_next_model_id++;
    }

    UInt RenderModelManager::CalculateLODLevel(Float distance) const
    {
        for (const auto& config : m_lod_configs)
        {
            if (distance <= config.distance_threshold)
            {
                return config.lod_level;
            }
        }
        
        // If distance is beyond all thresholds, return the highest LOD level
        if (!m_lod_configs.empty())
        {
            return m_lod_configs.back().lod_level;
        }
        
        return 0; // Default to highest detail
    }

} // namespace Dolas
