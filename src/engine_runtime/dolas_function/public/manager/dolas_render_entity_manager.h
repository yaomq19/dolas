#ifndef DOLAS_RENDER_ENTITY_MANAGER_H
#define DOLAS_RENDER_ENTITY_MANAGER_H

#include <unordered_map>
#include "dolas_hash.h"
#include "dolas_math.h"
namespace Dolas
{
    class AssetPath;
    class RenderEntity;
    class RenderEntityManager
    {
    public:
        RenderEntityManager();
        ~RenderEntityManager();
        bool Initialize();
        bool Clear();

		RenderEntityID CreateRenderEntityFromFile(const AssetPath& asset_path,
                                                  const Vector3& position,
                                                  const Quaternion& rotation,
                                                  const Vector3& scale);
		RenderEntity* GetRenderEntityByID(RenderEntityID render_entity_id);
        RenderEntity* GetRenderEntityByAssetPath(const AssetPath& asset_path);
    protected:
        std::unordered_map<RenderEntityID, RenderEntity*> m_render_entities;
    };// class RenderEntityManager
}// namespace Dolas
#endif // DOLAS_RENDER_ENTITY_MANAGER_H
