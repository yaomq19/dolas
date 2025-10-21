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
        for (const std::string model_name : scene_asset->model_names)
        {
			std::string model_path = PathUtils::GetModelSourceDir() + model_name;
			const aiScene* scene = importer.ReadFile(
				model_path,
				aiProcess_Triangulate |
				aiProcess_FlipUVs |
				aiProcess_CalcTangentSpace |
				aiProcess_GenNormals
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				LOG_ERROR("{0}", importer.GetErrorString());
				return;
			}

			// 这里需要具体实现将aiScene转换为RenderEntity的逻辑
        }
    }

} // namespace Dolas 