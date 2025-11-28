#ifndef DOLAS_RENDER_ENTITY_MANAGER_H
#define DOLAS_RENDER_ENTITY_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "common/dolas_hash.h"
namespace Dolas
{
    class RenderEntity;
    class RenderEntityManager
    {
    public:
        RenderEntityManager();
        ~RenderEntityManager();
        bool Initialize();
        bool Clear();

        RenderEntityID CreateRenderEntityFromFile(const std::string& render_entity_file_name);
        RenderEntity* GetRenderEntityByID(RenderEntityID render_entity_id);
        RenderEntity* GetRenderEntityByFileName(const std::string& render_entity_file_name);
    protected:
        std::unordered_map<RenderEntityID, RenderEntity*> m_render_entities;
    };// class RenderEntityManager
}// namespace Dolas
#endif // DOLAS_RENDER_ENTITY_MANAGER_H