#ifndef DOLAS_ENGINE_H
#define DOLAS_ENGINE_H
namespace Dolas
{
	class DolasRHI;
	class RenderPipeline;

	class DolasEngine
	{
	public:
		DolasEngine();
		~DolasEngine();

		bool Initialize();
		void Run();
		void Render();
		
		DolasRHI* m_rhi;
		RenderPipeline* m_render_pipeline;
	};
	extern DolasEngine g_dolas_engine;
}// namespace Dolas
#endif // DOLAS_ENGINE_H
