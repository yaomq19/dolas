#include "render/dolas_render_object.h"
#include "dolas_engine.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_entity.h"
#include "dolas_base.h"

namespace Dolas
{
    RenderObject::RenderObject()
    {
    }

    RenderObject::~RenderObject()
    {
    }

    void RenderObject::Draw(DolasRHI* rhi)
    {
        RenderEntity* render_entity = g_dolas_engine.m_render_entity_manager->GetRenderEntityByID(m_render_entity_id);
        if (render_entity)
        {
            render_entity->Draw(rhi);
        }
    }

    void RenderObject::SetRenderEntityID(RenderEntityID entity_id)
    {
        m_render_entity_id = entity_id;
    }

    RenderEntityID RenderObject::GetRenderEntityID() const
    {
        return m_render_entity_id;
    }

} // namespace Dolas
