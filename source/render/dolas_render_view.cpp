#include "render/dolas_render_view.h"
#include "render/dolas_render_camera.h"
#include "render/dolas_render_pipeline.h"
#include "render/dolas_render_resource.h"
#include "render/dolas_render_scene.h"
#include "core/dolas_rhi.h"
#include <iostream>
#include <chrono>

namespace Dolas
{
    RenderView::RenderView()
    {
        m_view_type = RenderViewType::MAIN_VIEW;
        m_viewport_x = 0;
        m_viewport_y = 0;
        m_viewport_width = 1920;
        m_viewport_height = 1080;
        m_enabled = true;
        m_priority = 0;

        m_rendered_entity_count = 0;
        m_culled_entity_count = 0;
        m_last_render_time = 0.0f;
    }

    RenderView::~RenderView()
    {

    }

    bool RenderView::Initialize()
    {
        std::cout << "RenderView::Initialize: Initializing render view: " << m_name << std::endl;
        return true;
    }

    bool RenderView::Clear()
    {
        // 清理组件（不释放，只是解除引用）
        m_camera.reset();
        m_pipeline.reset();
        m_resource.reset();
        m_scene.reset();

        // 重置统计信息
        ResetRenderStats();

        std::cout << "RenderView::Clear: Cleared render view: " << m_name << std::endl;
        return true;
    }

    void RenderView::SetCamera(std::shared_ptr<RenderCamera> camera)
    {
        m_camera = camera;
        if (camera)
        {
            std::cout << "RenderView::SetCamera: Set camera for view: " << m_name << std::endl;
        }
    }

    void RenderView::SetPipeline(std::shared_ptr<RenderPipeline> pipeline)
    {
        m_pipeline = pipeline;
        if (pipeline)
        {
            std::cout << "RenderView::SetPipeline: Set pipeline for view: " << m_name << std::endl;
        }
    }

    void RenderView::SetResource(std::shared_ptr<RenderResource> resource)
    {
        m_resource = resource;
        if (resource)
        {
            std::cout << "RenderView::SetResource: Set resource for view: " << m_name << std::endl;
        }
    }

    void RenderView::SetScene(std::shared_ptr<RenderScene> scene)
    {
        m_scene = scene;
        if (scene)
        {
            std::cout << "RenderView::SetScene: Set scene for view: " << m_name << std::endl;
        }
    }

    void RenderView::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_viewport_x = x;
        m_viewport_y = y;
        m_viewport_width = width;
        m_viewport_height = height;

        // 如果有相机，更新其宽高比
        if (m_camera && width > 0 && height > 0)
        {
            float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
            if (m_camera->GetCameraType() == CameraType::PERSPECTIVE)
            {
                m_camera->SetPerspective(m_camera->GetFOV(), aspect_ratio, 
                                       m_camera->GetNearPlane(), m_camera->GetFarPlane());
            }
        }

        std::cout << "RenderView::SetViewport: Set viewport (" << x << ", " << y 
                  << ", " << width << ", " << height << ") for view: " << m_name << std::endl;
    }

    void RenderView::GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) const
    {
        x = m_viewport_x;
        y = m_viewport_y;
        width = m_viewport_width;
        height = m_viewport_height;
    }

    bool RenderView::IsReadyToRender() const
    {
        if (!m_enabled)
        {
            return false;
        }

        if (!m_camera)
        {
            std::cerr << "RenderView::IsReadyToRender: No camera set for view: " << m_name << std::endl;
            return false;
        }

        if (!m_pipeline)
        {
            std::cerr << "RenderView::IsReadyToRender: No pipeline set for view: " << m_name << std::endl;
            return false;
        }

        if (!m_resource)
        {
            std::cerr << "RenderView::IsReadyToRender: No resource set for view: " << m_name << std::endl;
            return false;
        }

        if (!m_scene)
        {
            std::cerr << "RenderView::IsReadyToRender: No scene set for view: " << m_name << std::endl;
            return false;
        }

        return true;
    }

    void RenderView::Render(DolasRHI* rhi)
    {
        if (!IsReadyToRender() || !rhi)
        {
            return;
        }

        // RenderDoc/PIX Event: View scope
        std::wstring view_event_name = L"RenderView: ";
        view_event_name += std::wstring(m_name.begin(), m_name.end());
        UserAnnotationScope view_scope(rhi, view_event_name.c_str());

        auto start_time = std::chrono::high_resolution_clock::now();

        std::cout << "RenderView::Render: Starting render for view: " << m_name << std::endl;

        // 设置视口
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = static_cast<float>(m_viewport_x);
        viewport.TopLeftY = static_cast<float>(m_viewport_y);
        viewport.Width = static_cast<float>(m_viewport_width);
        viewport.Height = static_cast<float>(m_viewport_height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        rhi->m_d3d_immediate_context->RSSetViewports(1, &viewport);

        // 执行场景剔除和排序
        m_scene->CullAndSort(*m_camera);

        // 更新渲染统计
        auto visible_items = m_scene->GetVisibleRenderItems();
        m_rendered_entity_count = static_cast<uint32_t>(visible_items.size());
        m_culled_entity_count = static_cast<uint32_t>(m_scene->GetRenderEntityCount() - visible_items.size());

        // 执行渲染管线
        m_pipeline->Render(rhi);

        // 计算渲染时间
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        m_last_render_time = duration.count() / 1000.0f; // 转换为毫秒

        std::cout << "RenderView::Render: Completed render for view: " << m_name 
                  << " (Rendered: " << m_rendered_entity_count 
                  << ", Culled: " << m_culled_entity_count 
                  << ", Time: " << m_last_render_time << "ms)" << std::endl;
    }

    void RenderView::ResetRenderStats()
    {
        m_rendered_entity_count = 0;
        m_culled_entity_count = 0;
        m_last_render_time = 0.0f;
    }

    void RenderView::UpdateRenderStats()
    {
        // 这个方法可以用于更新更复杂的渲染统计信息
        // 目前在Render方法中直接更新统计信息
    }

} // namespace Dolas 