#ifndef TRIANGLE_RENDER_ENTITY_H
#define TRIANGLE_RENDER_ENTITY_H
#include "Function/Render/RenderEntity/RenderEntity.h"
namespace Dolas
{
    class TriangleRenderEntity : public RenderEntity
    {
    private:
        /* data */
    public:
        TriangleRenderEntity(/* args */);
        ~TriangleRenderEntity();

        virtual void Render() const override
        {
            // 渲染三角形
        }
    };
}


#endif // TRIANGLE_RENDER_ENTITY_H