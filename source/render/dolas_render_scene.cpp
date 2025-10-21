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

// ����Assimp������
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
		// ����3Dģ���ļ�
		const aiScene* scene = importer.ReadFile("path/to/your/model.obj",
			aiProcess_Triangulate |           // ȷ�������涼��������
			aiProcess_FlipUVs |              // ��תUV���꣨OpenGL��Ҫ��
			aiProcess_CalcTangentSpace |     // �������߿ռ�
			aiProcess_GenNormals             // ���ɷ��ߣ����û�еĻ���
		);

		// �������Ƿ�ɹ�
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            LOG_ERROR("{0}", importer.GetErrorString());
			return;
		}
    }

} // namespace Dolas 