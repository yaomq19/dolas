#ifndef DOLAS_RENDER_VIEW_H
#define DOLAS_RENDER_VIEW_H

#include <string>
#include <memory>

namespace Dolas
{
    class RenderCamera;
    class RenderPipeline;
    class RenderResource;
    class RenderScene;
    class DolasRHI;

    enum class RenderViewType
    {
        MAIN_VIEW,          // 主渲染视图
        SHADOW_VIEW,        // 阴影渲染视图
        REFLECTION_VIEW,    // 反射渲染视图
        REFRACTION_VIEW,    // 折射渲染视图
        CUBEMAP_VIEW,       // 立方体贴图渲染视图
        POST_PROCESS_VIEW   // 后处理渲染视图
    };

    class RenderView
    {
        friend class RenderViewManager;
    public:
        RenderView();
        ~RenderView();

        bool Initialize();
        bool Clear();

        // 组件设置
        void SetCamera(std::shared_ptr<RenderCamera> camera);
        void SetPipeline(std::shared_ptr<RenderPipeline> pipeline);
        void SetResource(std::shared_ptr<RenderResource> resource);
        void SetScene(std::shared_ptr<RenderScene> scene);

        // 组件获取
        std::shared_ptr<RenderCamera> GetCamera() const { return m_camera; }
        std::shared_ptr<RenderPipeline> GetPipeline() const { return m_pipeline; }
        std::shared_ptr<RenderResource> GetResource() const { return m_resource; }
        std::shared_ptr<RenderScene> GetScene() const { return m_scene; }

        // 渲染视图属性
        void SetViewType(RenderViewType type) { m_view_type = type; }
        RenderViewType GetViewType() const { return m_view_type; }
        
        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) const;

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        void SetPriority(int priority) { m_priority = priority; }
        int GetPriority() const { return m_priority; }

        // 渲染执行
        bool IsReadyToRender() const;
        void Render(DolasRHI* rhi);

        // 渲染统计
        void ResetRenderStats();
        uint32_t GetRenderedEntityCount() const { return m_rendered_entity_count; }
        uint32_t GetCulledEntityCount() const { return m_culled_entity_count; }
        float GetLastRenderTime() const { return m_last_render_time; }

        std::string m_name;

    private:
        void UpdateRenderStats();

        // 核心组件
        std::shared_ptr<RenderCamera> m_camera;
        std::shared_ptr<RenderPipeline> m_pipeline;
        std::shared_ptr<RenderResource> m_resource;
        std::shared_ptr<RenderScene> m_scene;

        // 渲染视图属性
        RenderViewType m_view_type;
        uint32_t m_viewport_x;
        uint32_t m_viewport_y;
        uint32_t m_viewport_width;
        uint32_t m_viewport_height;
        bool m_enabled;
        int m_priority; // 数值越小优先级越高

        // 渲染统计
        uint32_t m_rendered_entity_count;
        uint32_t m_culled_entity_count;
        float m_last_render_time;
    }; // class RenderView
} // namespace Dolas

#endif // DOLAS_RENDER_VIEW_H 