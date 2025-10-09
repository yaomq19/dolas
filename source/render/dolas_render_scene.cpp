#include "render/dolas_render_scene.h"
#include "render/dolas_render_entity.h"
#include "render/dolas_render_camera.h"
#include <iostream>
#include <algorithm>
#include <cfloat>
#include <DirectXMath.h>
using namespace DirectX;
#include "manager/dolas_asset_manager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// 创建Assimp导入器
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
		// 加载3D模型文件
		const aiScene* scene = importer.ReadFile("path/to/your/model.obj",
			aiProcess_Triangulate |           // 确保所有面都是三角形
			aiProcess_FlipUVs |              // 翻转UV坐标（OpenGL需要）
			aiProcess_CalcTangentSpace |     // 计算切线空间
			aiProcess_GenNormals             // 生成法线（如果没有的话）
		);

		// 检查加载是否成功
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}
    }

} // namespace Dolas 