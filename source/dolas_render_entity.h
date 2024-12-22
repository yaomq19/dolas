#ifndef DOLAS_RENDER_ENTITY_H
#define DOLAS_RENDER_ENTITY_H
#include "dolas_common.h"
#include "dolas_rhi.h"
namespace Dolas
{
class RenderEntity
{
public:
    RenderEntity();
    ~RenderEntity();
    virtual void render(RHI* rhi) = 0;
};
}
#endif