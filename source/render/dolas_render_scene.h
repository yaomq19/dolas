#ifndef DOLAS_RENDER_SCENE_H
#define DOLAS_RENDER_SCENE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>

namespace Dolas
{
    class RenderEntity;
    class RenderCamera;

    enum class RenderLayer
    {
        RenderLayer_BACKGROUND = 0,
        RenderLayer_OPAQUE = 1000,
        RenderLayer_ALPHA_TEST = 2000,
        RenderLayer_TRANSPARENT = 3000,
        RenderLayer_OVERLAY = 4000
    };

    struct RenderItem
    {
        std::shared_ptr<RenderEntity> entity;
        RenderLayer layer;
        float distance_to_camera;
        DirectX::XMFLOAT4X4 world_matrix;
        bool visible;

        RenderItem() : layer(RenderLayer::RenderLayer_OPAQUE), distance_to_camera(0.0f), visible(true)
        {
            DirectX::XMStoreFloat4x4(&world_matrix, DirectX::XMMatrixIdentity());
        }
    };

    class RenderScene
    {
        friend class RenderSceneManager;
    public:
        RenderScene();
        ~RenderScene();

        bool Initialize();
        bool Clear();

        // RenderEntity管理
        bool AddRenderEntity(const std::string& name, std::shared_ptr<RenderEntity> entity, RenderLayer layer = RenderLayer::RenderLayer_OPAQUE);
        bool RemoveRenderEntity(const std::string& name);
        std::shared_ptr<RenderEntity> GetRenderEntity(const std::string& name) const;
        bool HasRenderEntity(const std::string& name) const;

        // 批量添加
        void AddRenderEntities(const std::vector<std::pair<std::string, std::shared_ptr<RenderEntity>>>& entities, RenderLayer layer = RenderLayer::RenderLayer_OPAQUE);

        // 渲染项管理
        void UpdateRenderItem(const std::string& name, const DirectX::XMFLOAT4X4& world_matrix);
        void SetRenderItemVisible(const std::string& name, bool visible);
        void SetRenderItemLayer(const std::string& name, RenderLayer layer);

        // 场景剔除和排序
        void CullAndSort(RenderCamera& camera);
        std::vector<RenderItem*> GetVisibleRenderItems() const;
        std::vector<RenderItem*> GetRenderItemsByLayer(RenderLayer layer) const;

        // 场景统计
        size_t GetRenderEntityCount() const { return m_render_items.size(); }
        size_t GetVisibleRenderEntityCount() const;
        std::vector<std::string> GetRenderEntityNames() const;

        // 场景边界
        void CalculateSceneBounds();
        DirectX::XMFLOAT3 GetSceneCenter() const { return m_scene_center; }
        DirectX::XMFLOAT3 GetSceneExtents() const { return m_scene_extents; }
        float GetSceneRadius() const { return m_scene_radius; }

        std::string m_name;

    private:
        void SortRenderItems();
        float CalculateDistanceToCamera(const DirectX::XMFLOAT4X4& world_matrix, RenderCamera& camera);

        std::unordered_map<std::string, RenderItem> m_render_items;
        std::vector<RenderItem*> m_visible_render_items;
        
        // 场景边界信息
        DirectX::XMFLOAT3 m_scene_center;
        DirectX::XMFLOAT3 m_scene_extents;
        float m_scene_radius;
        bool m_bounds_dirty;
    }; // class RenderScene
} // namespace Dolas

#endif // DOLAS_RENDER_SCENE_H 