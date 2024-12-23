#ifndef DOLAS_SCENE_H
#define DOLAS_SCENE_H
#include <vector>
#include "dolas_common.h"
#include "dolas_render_entity.h"
#include "dolas_triangle.h"
namespace Dolas
{
class Scene
{
public:
    Scene();
    ~Scene();
    bool initialize();
    void tick(Float delta_time);
    void addRenderEntity(RenderEntity* render_entity);
    void removeRenderEntity(RenderEntity* render_entity);

    std::vector<RenderEntity*> m_render_entitys;
};
}
#endif