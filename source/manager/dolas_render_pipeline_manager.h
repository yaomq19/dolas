#ifndef DOLAS_RENDER_PIPELINE_MANAGER_H
#define DOLAS_RENDER_PIPELINE_MANAGER_H
#include <unordered_map>
#include <string>

namespace Dolas
{
    class RenderPipeline;
    class RenderPipelineManager
    {
    public:
        RenderPipelineManager();
        ~RenderPipelineManager();
        bool Initialize();
        bool Clear();
        RenderPipeline* GetMainRenderPipeline();
        RenderPipeline* GetRenderPipeline(const std::string& name);
    private:
        std::unordered_map<std::string, RenderPipeline*> m_render_pipelines;
    };// class RenderPipelineManager
}// namespace Dolas

#endif // DOLAS_RENDER_PIPELINE_MANAGER_H