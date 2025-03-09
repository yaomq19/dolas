#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H
#include <d3d11.h>
#include "Function/Render/TextureManager/TextureManager.h"
namespace Dolas
{
    class RenderPipeline
    {
    public:
        RenderPipeline();
        ~RenderPipeline();
        bool Initialize();
        void Render();
    private:

    };
}

#endif // RENDER_PIPELINE_H