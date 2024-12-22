#ifndef DOLAS_TRIANGLE_H
#define DOLAS_TRIANGLE_H
#include "dolas_common.h"
#include "dolas_render_entity.h"
namespace Dolas
{
class Triangle : public RenderEntity
{
public:
    Triangle();
    ~Triangle();
    virtual void render(RHI* rhi) override;
private:
    Float m_vertices[9] = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    Float m_colors[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    
};
}
#endif