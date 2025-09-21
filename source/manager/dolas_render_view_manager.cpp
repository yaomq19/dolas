#include "base/dolas_base.h"
#include "manager/dolas_render_view_manager.h"
#include "render/dolas_render_view.h"
#include "manager/dolas_render_pipeline_manager.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_resource_manager.h"
#include "render/dolas_render_pipeline.h"
#include "manager/dolas_render_scene_manager.h"
#include "manager/dolas_render_camera_manager.h"
#include "render/dolas_render_scene.h"
#include "render/dolas_render_camera.h"
namespace Dolas
{
    const static RenderViewID RENDER_VIEW_ID_MAIN = STRING_ID(main_render_view);
	const static RenderPipelineID RENDER_PIPELINE_ID_MAIN = STRING_ID(main_render_pipeline);
	const static RenderCameraID RENDER_CAMERA_ID_MAIN = STRING_ID(main_render_camera);
	const static RenderSceneID RENDER_SCENE_ID_MAIN = STRING_ID(main_render_scene);
	const static RenderResourceID RENDER_RESOURCE_ID_MAIN = STRING_ID(main_render_resource);

    RenderViewManager::RenderViewManager()
    {
    }

    RenderViewManager::~RenderViewManager()
    {
    }

    Bool RenderViewManager::Initialize()
    {
        Bool ret = true;
        ret &= CreateRenderViewByID(RENDER_VIEW_ID_MAIN, RENDER_CAMERA_ID_MAIN, RENDER_PIPELINE_ID_MAIN, RENDER_RESOURCE_ID_MAIN, RENDER_SCENE_ID_MAIN);
        return ret;
    }

    Bool RenderViewManager::Clear()
    {
        for (auto it = m_render_views.begin(); it != m_render_views.end(); ++it)
        {
            RenderView* view = it->second;
            if (view)
            {
                view->Clear();
                DOLAS_DELETE(view);
            }
        }
        m_render_views.clear();
        return true;
    }

    RenderView* RenderViewManager::GetMainRenderView()
    {
        if (m_render_views.find(RENDER_VIEW_ID_MAIN) == m_render_views.end()) return nullptr;
        return m_render_views[RENDER_VIEW_ID_MAIN];
    }

    RenderView* RenderViewManager::GetRenderView(RenderViewID id)
    {
        if (m_render_views.find(id) == m_render_views.end()) return nullptr;
        return m_render_views[id];
    }

    Bool RenderViewManager::CreateRenderViewByID(RenderViewID render_view_id, RenderCameraID render_camera_id, RenderPipelineID render_pipeline_id, RenderResourceID render_resource_id, RenderSceneID render_scene_id)
    {
        RenderView* render_view = DOLAS_NEW(RenderView);
        render_view->m_render_camera_id = render_camera_id;
        render_view->m_render_pipeline_id = render_pipeline_id;
        render_view->m_render_resource_id = render_resource_id;
        render_view->m_render_scene_id = render_scene_id;
        m_render_views[render_view_id] = render_view;

        DOLAS_RETURN_FALSE_IF_FALSE(render_view->Initialize());

        render_view->SetRenderResourceID(render_resource_id);
        render_view->SetRenderCameraID(render_camera_id);
        render_view->SetRenderSceneID(render_scene_id);
        render_view->SetRenderPipelineID(render_pipeline_id);

		RenderResourceManager* render_resource_manager = g_dolas_engine.m_render_resource_manager;
		DOLAS_RETURN_FALSE_IF_NULL(render_resource_manager);
		Bool ret = render_resource_manager->CreateRenderResourceByID(render_view->m_render_resource_id);
		DOLAS_RETURN_FALSE_IF_FALSE(ret);

        RenderSceneManager* render_scene_manager = g_dolas_engine.m_render_scene_manager;
		DOLAS_RETURN_FALSE_IF_NULL(render_scene_manager);
        ret = render_scene_manager->CreateRenderSceneByID(render_view->m_render_scene_id);
        DOLAS_RETURN_FALSE_IF_FALSE(ret);
        RenderScene* render_scene = render_scene_manager->GetRenderSceneByID(render_view->m_render_scene_id);
        DOLAS_RETURN_FALSE_IF_NULL(render_scene);
        render_scene->BuildFromFile("default.scene");

        RenderCameraManager* render_camera_manager = g_dolas_engine.m_render_camera_manager;
		DOLAS_RETURN_FALSE_IF_NULL(render_camera_manager);
        ret = render_camera_manager->CreateRenderCameraByID(render_view->m_render_camera_id, "default.camera");
        DOLAS_RETURN_FALSE_IF_FALSE(ret);
        RenderCamera* render_camera = render_camera_manager->GetRenderCameraByID(render_view->m_render_camera_id);
        DOLAS_RETURN_FALSE_IF_NULL(render_camera);

        RenderPipelineManager* render_pipeline_manager = g_dolas_engine.m_render_pipeline_manager;
		DOLAS_RETURN_FALSE_IF_NULL(render_pipeline_manager);
        ret = render_pipeline_manager->CreateRenderPipelineByID(render_view->m_render_pipeline_id);
        DOLAS_RETURN_FALSE_IF_FALSE(ret);
        RenderPipeline* render_pipeline = render_pipeline_manager->GetRenderPipelineByID(render_view->m_render_pipeline_id);
        DOLAS_RETURN_FALSE_IF_NULL(render_pipeline);
        render_pipeline->SetRenderViewID(render_view_id);
        return true;
    }
} // namespace Dolas


