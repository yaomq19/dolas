#ifndef DOLAS_RENDER_MODEL_MANAGER_H
#define DOLAS_RENDER_MODEL_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "common/dolas_hash.h"
#include "core/dolas_math.h"

namespace Dolas
{
    class RenderModel;
    
    /**
     * @brief Manager for all RenderModel instances in the scene
     * 
     * The RenderModelManager handles:
     * - Creating and destroying render models
     * - Managing model lifetime
     * - Providing lookup functionality
     * - LOD management and culling
     * - Bulk operations on models
     */
    class RenderModelManager
    {
    public:
        RenderModelManager();
        ~RenderModelManager();
        
        bool Initialize();
        bool Clear();
        void Update(Float delta_time);
        
        // Model creation and management
        RenderModelID CreateRenderModel();
        RenderModelID CreateRenderModel(const std::string& name);
        RenderModelID CreateRenderModelFromObjects(const std::vector<RenderObjectID>& object_ids);
        RenderModelID CreateRenderModelFromObjects(const std::vector<RenderObjectID>& object_ids, const std::string& name);
        
        bool DestroyRenderModel(RenderModelID model_id);
        
        // Model lookup
        RenderModel* GetRenderModelByID(RenderModelID model_id);
        RenderModel* GetRenderModelByName(const std::string& name);
        
        // Bulk operations
        void DrawVisibleModels(class DolasRHI* rhi, const class RenderCamera* camera);
        void SetAllModelsVisible(bool visible);
        void UpdateLOD(const Vector3& camera_position);
        
        // Scene management helpers
        void ClearAllModels();
        UInt GetModelCount() const;
        std::vector<RenderModelID> GetAllModelIDs() const;
        
        // LOD configuration
        struct LODConfig
        {
            Float distance_threshold;
            UInt lod_level;
        };
        
        void SetLODConfig(const std::vector<LODConfig>& lod_configs);
        void EnableAutoLOD(bool enable);
        
    protected:
        std::unordered_map<RenderModelID, RenderModel*> m_render_models;
        std::unordered_map<std::string, RenderModelID> m_name_to_id_map;
        RenderModelID m_next_model_id;
        
        // LOD management
        std::vector<LODConfig> m_lod_configs;
        bool m_auto_lod_enabled;
        
        RenderModelID GenerateNewModelID();
        UInt CalculateLODLevel(Float distance) const;
    };
    
} // namespace Dolas

#endif // DOLAS_RENDER_MODEL_MANAGER_H
