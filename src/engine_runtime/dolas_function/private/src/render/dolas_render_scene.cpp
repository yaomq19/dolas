#include "render/dolas_render_scene.h"
#include "render/dolas_render_entity.h"
#include "render/dolas_render_camera.h"
#include <iostream>
#include <algorithm>
#include <cfloat>
#include <DirectXMath.h>
using namespace DirectX;
#include "dolas_asset_manager.h"
#include "dolas_log_system_manager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "dolas_paths.h"
#include "dolas_engine.h"
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

    // BuildFromAsset 已废弃：RenderSceneManager 直接构建 m_render_entities

} // namespace Dolas 