#ifndef DOLAS_RENDER_OBJECT_MANAGER_H
#define DOLAS_RENDER_OBJECT_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "dolas_hash.h"
#include "dolas_math.h"

namespace Dolas
{
    class RenderObject;
    
    /**
     * @brief Manager for all RenderObject instances in the scene
     * 
     * The RenderObjectManager handles:
     * - Creating and destroying render objects
     * - Managing object lifetime
     * - Providing lookup functionality
     * - Bulk operations on objects (update, culling, etc.)
     */
    class RenderObjectManager
    {
    public:
        RenderObjectManager();
        ~RenderObjectManager();
        
        bool Initialize();
        bool Clear();
        void Update(Float delta_time);
        
        // Object creation and management
        RenderObjectID CreateRenderObject();
        RenderObjectID CreateRenderObject(const std::string& name);
        RenderObjectID CreateRenderObjectFromEntity(const std::string& entity_file_name);
        RenderObjectID CreateRenderObjectFromEntity(const std::string& entity_file_name, const Vector3& position);
        RenderObjectID CreateRenderObjectFromEntity(const std::string& entity_file_name, 
                                                   const Vector3& position, 
                                                   const Vector3& rotation, 
                                                   const Vector3& scale);
        
        bool DestroyRenderObject(RenderObjectID object_id);
        
        // Object lookup
        RenderObject* GetRenderObjectByID(RenderObjectID object_id);
        RenderObject* GetRenderObjectByName(const std::string& name);
        
        // Bulk operations
        void DrawAllObjects(class DolasRHI* rhi);
        void SetAllObjectsVisible(bool visible);
        
        // Scene management helpers
        void ClearAllObjects();
        UInt GetObjectCount() const;
        std::vector<RenderObjectID> GetAllObjectIDs() const;
        
    protected:
        std::unordered_map<RenderObjectID, RenderObject*> m_render_objects;
        std::unordered_map<std::string, RenderObjectID> m_name_to_id_map;
        RenderObjectID m_next_object_id;
        
        RenderObjectID GenerateNewObjectID();
    };
    
} // namespace Dolas

#endif // DOLAS_RENDER_OBJECT_MANAGER_H
