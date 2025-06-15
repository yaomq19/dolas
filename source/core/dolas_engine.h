#ifndef DOLAS_ENGINE_H
#define DOLAS_ENGINE_H
namespace Dolas
{
	class DolasRHI;
	class MeshManager;
	class MaterialManager;
	class RenderEntityManager;
	class RenderPipelineManager;

	class DolasEngine
	{
	public:
		DolasEngine();
		~DolasEngine();

		bool Initialize();
		void Run();
		void Update();
		void Render();
		
		DolasRHI* m_rhi;
		RenderPipelineManager* m_render_pipeline_manager;
		MeshManager* m_mesh_manager;
		MaterialManager* m_material_manager;
		RenderEntityManager* m_render_entity_manager;
	};
	extern DolasEngine g_dolas_engine;
}// namespace Dolas
#endif // DOLAS_ENGINE_H
