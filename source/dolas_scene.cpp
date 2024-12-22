#include "dolas_scene.h"
namespace Dolas
{
Scene::Scene()
{

}

Scene::~Scene()
{

}

bool Scene::initialize()
{
    m_render_entitys.push_back(new Triangle());
    return true;
}

void Scene::tick(Float delta_time)
{
    // Do nothing
}
}