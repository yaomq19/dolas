#ifndef DOLAS_RENDERER_H
#define DOLAS_RENDERER_H
namespace Dolas {
class Renderer {
public:
    virtual void render(RenderScene* render_scene) = 0;
};
}
#endif