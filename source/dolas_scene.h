#ifndef DOLAS_SCENE_H
#define DOLAS_SCENE_H
#include "dolas_common.h"
namespace Dolas
{
class Scene
{
public:
    Scene();
    ~Scene();
    void tick(Float delta_time);
};
}
#endif