#ifndef DOLAS_RENDER_CAMERA_H
#define DOLAS_RENDER_CAMERA_H

#include <DirectXMath.h>
#include <string>
#include "common/dolas_hash.h"

namespace Dolas
{
    enum class CameraType
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    class RenderCamera
    {
        friend class RenderCameraManager;
    public:
        RenderCamera();
        ~RenderCamera();

		Bool Initialize();
        // 相机设置
        void SetPosition(const DirectX::XMFLOAT3& position);
        void SetRotation(const DirectX::XMFLOAT3& rotation); // pitch, yaw, roll (弧度)
        void SetTarget(const DirectX::XMFLOAT3& target);
        void SetUpVector(const DirectX::XMFLOAT3& up);

        // 透视投影设置
        void SetPerspective(float fov_y, float aspect_ratio, float near_plane, float far_plane);
        // 正交投影设置
        void SetOrthographic(float width, float height, float near_plane, float far_plane);

        // 相机移动
        void MoveForward(float distance);
        void MoveRight(float distance);
        void MoveUp(float distance);
        void Rotate(float pitch, float yaw, float roll);

        // 矩阵获取
        DirectX::XMMATRIX GetViewMatrix();
        DirectX::XMMATRIX GetProjectionMatrix();
        DirectX::XMMATRIX GetViewProjectionMatrix();

        // 向量获取
        DirectX::XMFLOAT3 GetPosition() const { return m_position; }
        DirectX::XMFLOAT3 GetRotation() const { return m_rotation; }
        DirectX::XMFLOAT3 GetForwardVector() const;
        DirectX::XMFLOAT3 GetRightVector() const;
        DirectX::XMFLOAT3 GetUpVector() const;

        // 投影参数获取
        CameraType GetCameraType() const { return m_camera_type; }
        float GetFOV() const { return m_fov_y; }
        float GetAspectRatio() const { return m_aspect_ratio; }
        float GetNearPlane() const { return m_near_plane; }
        float GetFarPlane() const { return m_far_plane; }

        // 视锥体检测
        bool IsPointInFrustum(const DirectX::XMFLOAT3& point);
        bool IsSphereInFrustum(const DirectX::XMFLOAT3& center, float radius);

        std::string m_name;

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
        void UpdateFrustumPlanes();

        // 相机变换
        DirectX::XMFLOAT3 m_position;
        DirectX::XMFLOAT3 m_rotation; // pitch, yaw, roll
        DirectX::XMFLOAT3 m_target;
        DirectX::XMFLOAT3 m_up;

        // 投影参数
        CameraType m_camera_type;
        float m_fov_y;           // 透视投影：垂直视野角度（弧度）
        float m_aspect_ratio;    // 宽高比
        float m_near_plane;      // 近裁剪面
        float m_far_plane;       // 远裁剪面
        float m_ortho_width;     // 正交投影：宽度
        float m_ortho_height;    // 正交投影：高度

        // 缓存的矩阵
        DirectX::XMFLOAT4X4 m_view_matrix;
        DirectX::XMFLOAT4X4 m_projection_matrix;
        bool m_view_matrix_dirty;
        bool m_projection_matrix_dirty;

        // 视锥体平面（用于视锥体剔除）
        DirectX::XMFLOAT4 m_frustum_planes[6]; // left, right, top, bottom, near, far
        bool m_frustum_dirty;
    }; // class RenderCamera
} // namespace Dolas

#endif // DOLAS_RENDER_CAMERA_H 