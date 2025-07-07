#include "render/dolas_render_scene.h"
#include "render/dolas_render_entity.h"
#include "render/dolas_render_camera.h"
#include <iostream>
#include <algorithm>
#include <cfloat>
#include <DirectXMath.h>
using namespace DirectX;

namespace Dolas
{
    RenderScene::RenderScene()
    {
        m_scene_center = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_scene_extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_scene_radius = 0.0f;
        m_bounds_dirty = true;
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
        m_render_items.clear();
        m_visible_render_items.clear();
        
        m_scene_center = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_scene_extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_scene_radius = 0.0f;
        m_bounds_dirty = true;

        std::cout << "RenderScene::Clear: Scene cleared: " << m_name << std::endl;
        return true;
    }

    bool RenderScene::AddRenderEntity(const std::string& name, std::shared_ptr<RenderEntity> entity, RenderLayer layer)
    {
        if (!entity)
        {
            std::cerr << "RenderScene::AddRenderEntity: Entity is null for name: " << name << std::endl;
            return false;
        }

        if (m_render_items.find(name) != m_render_items.end())
        {
            std::cerr << "RenderScene::AddRenderEntity: Entity with name '" << name << "' already exists" << std::endl;
            return false;
        }

        RenderItem item;
        item.entity = entity;
        item.layer = layer;
        item.visible = true;
        item.distance_to_camera = 0.0f;
        XMStoreFloat4x4(&item.world_matrix, XMMatrixIdentity());

        m_render_items[name] = item;
        m_bounds_dirty = true;

        std::cout << "RenderScene::AddRenderEntity: Added entity: " << name << " to layer " << static_cast<int>(layer) << std::endl;
        return true;
    }

    bool RenderScene::RemoveRenderEntity(const std::string& name)
    {
        auto it = m_render_items.find(name);
        if (it != m_render_items.end())
        {
            m_render_items.erase(it);
            m_bounds_dirty = true;
            std::cout << "RenderScene::RemoveRenderEntity: Removed entity: " << name << std::endl;
            return true;
        }
        
        std::cerr << "RenderScene::RemoveRenderEntity: Entity not found: " << name << std::endl;
        return false;
    }

    std::shared_ptr<RenderEntity> RenderScene::GetRenderEntity(const std::string& name) const
    {
        auto it = m_render_items.find(name);
        if (it != m_render_items.end())
        {
            return it->second.entity;
        }
        return nullptr;
    }

    bool RenderScene::HasRenderEntity(const std::string& name) const
    {
        return m_render_items.find(name) != m_render_items.end();
    }

    void RenderScene::AddRenderEntities(const std::vector<std::pair<std::string, std::shared_ptr<RenderEntity>>>& entities, RenderLayer layer)
    {
        for (const auto& pair : entities)
        {
            AddRenderEntity(pair.first, pair.second, layer);
        }
    }

    void RenderScene::UpdateRenderItem(const std::string& name, const XMFLOAT4X4& world_matrix)
    {
        auto it = m_render_items.find(name);
        if (it != m_render_items.end())
        {
            it->second.world_matrix = world_matrix;
            m_bounds_dirty = true;
        }
        else
        {
            std::cerr << "RenderScene::UpdateRenderItem: Entity not found: " << name << std::endl;
        }
    }

    void RenderScene::SetRenderItemVisible(const std::string& name, bool visible)
    {
        auto it = m_render_items.find(name);
        if (it != m_render_items.end())
        {
            it->second.visible = visible;
        }
        else
        {
            std::cerr << "RenderScene::SetRenderItemVisible: Entity not found: " << name << std::endl;
        }
    }

    void RenderScene::SetRenderItemLayer(const std::string& name, RenderLayer layer)
    {
        auto it = m_render_items.find(name);
        if (it != m_render_items.end())
        {
            it->second.layer = layer;
        }
        else
        {
            std::cerr << "RenderScene::SetRenderItemLayer: Entity not found: " << name << std::endl;
        }
    }

    void RenderScene::CullAndSort(RenderCamera& camera)
    {
        m_visible_render_items.clear();

        // 视锥体剔除和距离计算
        for (auto& pair : m_render_items)
        {
            RenderItem& item = pair.second;
            
            if (!item.visible || !item.entity)
            {
                continue;
            }

            // 简单的视锥体剔除 - 检查物体中心点
            XMVECTOR world_pos = XMVectorSet(
                item.world_matrix._41,
                item.world_matrix._42,
                item.world_matrix._43,
                1.0f
            );
            
            XMFLOAT3 world_pos_float3;
            XMStoreFloat3(&world_pos_float3, world_pos);

            // 使用简单的球体剔除（假设半径为1.0f）
            if (camera.IsSphereInFrustum(world_pos_float3, 1.0f))
            {
                // 计算到相机的距离
                item.distance_to_camera = CalculateDistanceToCamera(item.world_matrix, camera);
                m_visible_render_items.push_back(&item);
            }
        }

        // 排序渲染项
        SortRenderItems();

        std::cout << "RenderScene::CullAndSort: " << m_visible_render_items.size() 
                  << " visible items out of " << m_render_items.size() << " total items" << std::endl;
    }

    std::vector<RenderItem*> RenderScene::GetVisibleRenderItems() const
    {
        return m_visible_render_items;
    }

    std::vector<RenderItem*> RenderScene::GetRenderItemsByLayer(RenderLayer layer) const
    {
        std::vector<RenderItem*> layer_items;
        for (RenderItem* item : m_visible_render_items)
        {
            if (item && item->layer == layer)
            {
                layer_items.push_back(item);
            }
        }
        return layer_items;
    }

    size_t RenderScene::GetVisibleRenderEntityCount() const
    {
        return m_visible_render_items.size();
    }

    std::vector<std::string> RenderScene::GetRenderEntityNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_render_items.size());
        for (const auto& pair : m_render_items)
        {
            names.push_back(pair.first);
        }
        return names;
    }

    void RenderScene::CalculateSceneBounds()
    {
        if (!m_bounds_dirty || m_render_items.empty())
        {
            return;
        }

        XMFLOAT3 min_bounds(FLT_MAX, FLT_MAX, FLT_MAX);
        XMFLOAT3 max_bounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const auto& pair : m_render_items)
        {
            const RenderItem& item = pair.second;
            
            // 获取世界位置
            XMFLOAT3 world_pos(item.world_matrix._41, item.world_matrix._42, item.world_matrix._43);
            
            // 更新边界框
            min_bounds.x = (std::min)(min_bounds.x, world_pos.x);
            min_bounds.y = (std::min)(min_bounds.y, world_pos.y);
            min_bounds.z = (std::min)(min_bounds.z, world_pos.z);
            
            max_bounds.x = (std::max)(max_bounds.x, world_pos.x);
            max_bounds.y = (std::max)(max_bounds.y, world_pos.y);
            max_bounds.z = (std::max)(max_bounds.z, world_pos.z);
        }

        // 计算场景中心
        m_scene_center.x = (min_bounds.x + max_bounds.x) * 0.5f;
        m_scene_center.y = (min_bounds.y + max_bounds.y) * 0.5f;
        m_scene_center.z = (min_bounds.z + max_bounds.z) * 0.5f;

        // 计算场景范围
        m_scene_extents.x = (max_bounds.x - min_bounds.x) * 0.5f;
        m_scene_extents.y = (max_bounds.y - min_bounds.y) * 0.5f;
        m_scene_extents.z = (max_bounds.z - min_bounds.z) * 0.5f;

        // 计算场景半径
        XMVECTOR center = XMLoadFloat3(&m_scene_center);
        XMVECTOR extents = XMLoadFloat3(&m_scene_extents);
        m_scene_radius = XMVectorGetX(XMVector3Length(extents));

        m_bounds_dirty = false;

        std::cout << "RenderScene::CalculateSceneBounds: Center(" << m_scene_center.x << ", " 
                  << m_scene_center.y << ", " << m_scene_center.z << "), Radius: " << m_scene_radius << std::endl;
    }

    void RenderScene::SortRenderItems()
    {
        std::sort(m_visible_render_items.begin(), m_visible_render_items.end(),
            [](const RenderItem* a, const RenderItem* b)
            {
                if (a->layer != b->layer)
                {
                    return static_cast<int>(a->layer) < static_cast<int>(b->layer);
                }

                if (a->layer == Dolas::RenderLayer::RenderLayer_TRANSPARENT)
                {
                    return a->distance_to_camera > b->distance_to_camera;
                }
                else
                {
                    return a->distance_to_camera < b->distance_to_camera;
                }
            });
    }

    float RenderScene::CalculateDistanceToCamera(const XMFLOAT4X4& world_matrix, RenderCamera& camera)
    {
        XMVECTOR world_pos = XMVectorSet(world_matrix._41, world_matrix._42, world_matrix._43, 1.0f);
        XMVECTOR camera_pos = XMLoadFloat3(&camera.GetPosition());
        XMVECTOR distance_vec = XMVectorSubtract(world_pos, camera_pos);
        return XMVectorGetX(XMVector3Length(distance_vec));
    }

} // namespace Dolas 