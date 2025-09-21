#include "render/dolas_render_model.h"
#include "render/dolas_render_object.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_object_manager.h"
#include "render/dolas_render_camera.h"
#include "base/dolas_base.h"
#include "manager/dolas_render_entity_manager.h"
#include "render/dolas_render_entity.h"
namespace Dolas
{
    RenderModel::RenderModel()
        : m_model_id(RENDER_MODEL_ID_EMPTY)
    {
    }

    RenderModel::~RenderModel()
    {
    }

    void RenderModel::Draw(class DolasRHI* rhi, Transform transform)
    {
        RenderEntity* render_entity = g_dolas_engine.m_render_entity_manager->GetRenderEntityByID(m_render_entity_id);
        if (render_entity)
        {
            render_entity->Draw(rhi, transform);
        }
    }

} // namespace Dolas
