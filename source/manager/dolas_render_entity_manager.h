#ifndef DOLAS_RENDER_ENTITY_MANAGER_H
#define DOLAS_RENDER_ENTITY_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>

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

        std::shared_ptr<RenderEntity> GetOrCreateRenderEntity(const std::string& file_name);

    private:
        std::shared_ptr<RenderEntity> CreateRenderEntity(const std::string& file_name);
        std::unordered_map<std::string, std::shared_ptr<RenderEntity>> m_render_entities;
    };// class RenderEntityManager
}// namespace Dolas
#endif // DOLAS_RENDER_ENTITY_MANAGER_H