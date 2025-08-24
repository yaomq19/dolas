#ifndef DOLAS_ENGINE_H
#define DOLAS_ENGINE_H
namespace Dolas
{
	class DolasEngine
	{
	public:
		DolasEngine();
		~DolasEngine();

		bool Initialize();
		void Clear();
		void Run();
		void Update();
		void Render();
		void Test();
		
		class DolasRHI* m_rhi;
		class RenderPipelineManager* m_render_pipeline_manager;
		class MeshManager* m_mesh_manager;
		class MaterialManager* m_material_manager;
		class RenderEntityManager* m_render_entity_manager;
		class ShaderManager* m_shader_manager;
		class AssetManager* m_asset_manager;
		class TextureManager* m_texture_manager;
		class RenderResourceManager* m_render_resource_manager;
		class RenderPrimitiveManager* m_render_primitive_manager;
		class BufferManager* m_buffer_manager;
		class TestManager* m_test_manager;
		class InputLayoutManager* m_input_layout_manager;

		class InputLayoutManager* GetInputLayoutManager() { return m_input_layout_manager; }
	};
	extern DolasEngine g_dolas_engine;
}// namespace Dolas
#endif // DOLAS_ENGINE_H
