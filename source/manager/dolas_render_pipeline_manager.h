#ifndef DOLAS_RENDER_PIPELINE_MANAGER_H
#define DOLAS_RENDER_PIPELINE_MANAGER_H
#include <unordered_map>
#include <string>
#include "common/dolas_hash.h"
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
        RenderPipeline* GetRenderPipelineByID(RenderPipelineID id);
		Bool CreateRenderPipelineByID(RenderPipelineID id);

		void DisplayWorldCoordinateSystem();
    private:
        std::unordered_map<RenderPipelineID, RenderPipeline*> m_render_pipelines;
    };// class RenderPipelineManager
}// namespace Dolas

#endif // DOLAS_RENDER_PIPELINE_MANAGER_H