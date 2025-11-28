#include "render/dolas_render_scene.h"
#include "render/dolas_render_entity.h"
#include "render/dolas_render_camera.h"
#include <iostream>
#include <algorithm>
#include <cfloat>
#include <DirectXMath.h>
using namespace DirectX;
#include "manager/dolas_asset_manager.h"
#include "manager/dolas_log_system_manager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "base/dolas_paths.h"
#include "core/dolas_engine.h"
#include "manager/dolas_render_entity_manager.h"
Assimp::Importer importer;

namespace Dolas
{
    RenderScene::RenderScene()
    {

    }

    RenderScene::~RenderScene()
    {

    }

    bool RenderScene::Initialize()
    {
        return true;
    }

    bool RenderScene::Clear()
    {
        return true;
    }

    void RenderScene::BuildFromAsset(SceneAsset* scene_asset)
    {
   //     for (const std::string model_name : scene_asset->model_names)
   //     {
			//std::string model_path = PathUtils::GetModelSourceDir() + model_name;
			//const aiScene* ai_scene = importer.ReadFile(
			//	model_path,
			//	aiProcess_Triangulate |
			//	aiProcess_FlipUVs |
			//	aiProcess_CalcTangentSpace |
			//	aiProcess_GenNormals
			//);

			//if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode) {
			//	LOG_ERROR("{0}", importer.GetErrorString());
			//	return;
			//}

			//// 这里需要具体实现将aiScene转换为RenderEntity的逻辑
   //     }

		for (const SceneEntity& scene_entity : scene_asset->entities)
		{
			RenderEntityID render_entity_id = g_dolas_engine.m_render_entity_manager->CreateRenderEntityFromFile(scene_entity.entity_file);
			if (render_entity_id != RENDER_ENTITY_ID_EMPTY)
			{
				m_render_entities.push_back(render_entity_id);
			}
		}
    }

} // namespace Dolas 