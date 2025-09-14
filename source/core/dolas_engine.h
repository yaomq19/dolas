#ifndef DOLAS_ENGINE_H
#define DOLAS_ENGINE_H
#include "base/dolas_base.h"
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
		void Update(Float delta_time);
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
		class RenderStateManager* m_render_state_manager;
		class RenderViewManager* m_render_view_manager;
		class RenderCameraManager* m_render_camera_manager;
		class RenderSceneManager* m_render_scene_manager;
		class InputManager* m_input_manager;
	};
	extern DolasEngine g_dolas_engine;
}// namespace Dolas
#endif // DOLAS_ENGINE_H
