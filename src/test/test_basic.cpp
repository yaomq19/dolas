#include "render_engine/render_scene/dolas_render_scene.h"
#include "render_engine/renderer/dolas_renderer.h"
#include "render_engine/renderer/d3d11/dolas_renderer_d3d11.h"
int main()
{
    Dolas::RenderScene* scene;
    Dolas::Renderer* renderer = new Dolas::Renderer_D3D11();
    renderer->render(scene);
    return 0;
}