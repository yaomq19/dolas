#include "render/dolas_render_camera.h"
#include <iostream>

using namespace DirectX;

namespace Dolas
{
    RenderCamera::RenderCamera()
    {
        m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
        m_target = XMFLOAT3(0.0f, 0.0f, 1.0f);
        m_up = XMFLOAT3(0.0f, 1.0f, 0.0f);

        m_camera_type = CameraType::PERSPECTIVE;
        m_fov_y = XM_PIDIV4; // 45度
        m_aspect_ratio = 16.0f / 9.0f;
        m_near_plane = 0.1f;
        m_far_plane = 1000.0f;
        m_ortho_width = 10.0f;
        m_ortho_height = 10.0f;

        XMStoreFloat4x4(&m_view_matrix, XMMatrixIdentity());
        XMStoreFloat4x4(&m_projection_matrix, XMMatrixIdentity());
        
        m_view_matrix_dirty = true;
        m_projection_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    RenderCamera::~RenderCamera()
    {

    }

    void RenderCamera::SetPosition(const XMFLOAT3& position)
    {
        m_position = position;
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::SetRotation(const XMFLOAT3& rotation)
    {
        m_rotation = rotation;
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::SetTarget(const XMFLOAT3& target)
    {
        m_target = target;
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::SetUpVector(const XMFLOAT3& up)
    {
        m_up = up;
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::SetPerspective(float fov_y, float aspect_ratio, float near_plane, float far_plane)
    {
        m_camera_type = CameraType::PERSPECTIVE;
        m_fov_y = fov_y;
        m_aspect_ratio = aspect_ratio;
        m_near_plane = near_plane;
        m_far_plane = far_plane;
        m_projection_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::SetOrthographic(float width, float height, float near_plane, float far_plane)
    {
        m_camera_type = CameraType::ORTHOGRAPHIC;
        m_ortho_width = width;
        m_ortho_height = height;
        m_near_plane = near_plane;
        m_far_plane = far_plane;
        m_projection_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::MoveForward(float distance)
    {
        XMVECTOR forward = XMLoadFloat3(&GetForwardVector());
        XMVECTOR position = XMLoadFloat3(&m_position);
        position = XMVectorAdd(position, XMVectorScale(forward, distance));
        XMStoreFloat3(&m_position, position);
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::MoveRight(float distance)
    {
        XMVECTOR right = XMLoadFloat3(&GetRightVector());
        XMVECTOR position = XMLoadFloat3(&m_position);
        position = XMVectorAdd(position, XMVectorScale(right, distance));
        XMStoreFloat3(&m_position, position);
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::MoveUp(float distance)
    {
        XMVECTOR up = XMLoadFloat3(&GetUpVector());
        XMVECTOR position = XMLoadFloat3(&m_position);
        position = XMVectorAdd(position, XMVectorScale(up, distance));
        XMStoreFloat3(&m_position, position);
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    void RenderCamera::Rotate(float pitch, float yaw, float roll)
    {
        m_rotation.x += pitch;
        m_rotation.y += yaw;
        m_rotation.z += roll;
        
        // 限制pitch角度防止翻转
        if (m_rotation.x > XM_PIDIV2 - 0.01f)
            m_rotation.x = XM_PIDIV2 - 0.01f;
        if (m_rotation.x < -XM_PIDIV2 + 0.01f)
            m_rotation.x = -XM_PIDIV2 + 0.01f;
            
        m_view_matrix_dirty = true;
        m_frustum_dirty = true;
    }

    XMMATRIX RenderCamera::GetViewMatrix()
    {
        if (m_view_matrix_dirty)
        {
            UpdateViewMatrix();
        }
        return XMLoadFloat4x4(&m_view_matrix);
    }

    XMMATRIX RenderCamera::GetProjectionMatrix()
    {
        if (m_projection_matrix_dirty)
        {
            UpdateProjectionMatrix();
        }
        return XMLoadFloat4x4(&m_projection_matrix);
    }

    XMMATRIX RenderCamera::GetViewProjectionMatrix()
    {
        return XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());
    }

    XMFLOAT3 RenderCamera::GetForwardVector() const
    {
        XMMATRIX rotation_matrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
        XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotation_matrix);
        XMFLOAT3 result;
        XMStoreFloat3(&result, forward);
        return result;
    }

    XMFLOAT3 RenderCamera::GetRightVector() const
    {
        XMMATRIX rotation_matrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
        XMVECTOR right = XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotation_matrix);
        XMFLOAT3 result;
        XMStoreFloat3(&result, right);
        return result;
    }

    XMFLOAT3 RenderCamera::GetUpVector() const
    {
        XMMATRIX rotation_matrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
        XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rotation_matrix);
        XMFLOAT3 result;
        XMStoreFloat3(&result, up);
        return result;
    }

    bool RenderCamera::IsPointInFrustum(const XMFLOAT3& point)
    {
        if (m_frustum_dirty)
        {
            UpdateFrustumPlanes();
        }

        XMVECTOR point_vec = XMLoadFloat3(&point);
        
        for (int i = 0; i < 6; ++i)
        {
            XMVECTOR plane = XMLoadFloat4(&m_frustum_planes[i]);
            float distance = XMVectorGetX(XMPlaneDotCoord(plane, point_vec));
            if (distance < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    bool RenderCamera::IsSphereInFrustum(const XMFLOAT3& center, float radius)
    {
        if (m_frustum_dirty)
        {
            UpdateFrustumPlanes();
        }

        XMVECTOR center_vec = XMLoadFloat3(&center);
        
        for (int i = 0; i < 6; ++i)
        {
            XMVECTOR plane = XMLoadFloat4(&m_frustum_planes[i]);
            float distance = XMVectorGetX(XMPlaneDotCoord(plane, center_vec));
            if (distance < -radius)
            {
                return false;
            }
        }
        return true;
    }

    void RenderCamera::UpdateViewMatrix()
    {
        XMVECTOR position = XMLoadFloat3(&m_position);
        XMVECTOR forward = XMLoadFloat3(&GetForwardVector());
        XMVECTOR up = XMLoadFloat3(&GetUpVector());
        
        XMVECTOR target = XMVectorAdd(position, forward);
        XMMATRIX view = XMMatrixLookAtLH(position, target, up);
        
        XMStoreFloat4x4(&m_view_matrix, view);
        m_view_matrix_dirty = false;
    }

    void RenderCamera::UpdateProjectionMatrix()
    {
        XMMATRIX projection;
        
        if (m_camera_type == CameraType::PERSPECTIVE)
        {
            projection = XMMatrixPerspectiveFovLH(m_fov_y, m_aspect_ratio, m_near_plane, m_far_plane);
        }
        else
        {
            projection = XMMatrixOrthographicLH(m_ortho_width, m_ortho_height, m_near_plane, m_far_plane);
        }
        
        XMStoreFloat4x4(&m_projection_matrix, projection);
        m_projection_matrix_dirty = false;
    }

    void RenderCamera::UpdateFrustumPlanes()
    {
        XMMATRIX view_projection = GetViewProjectionMatrix();
        
        // 提取视锥体平面
        XMFLOAT4X4 vp_matrix;
        XMStoreFloat4x4(&vp_matrix, view_projection);
        
        // Left plane
        m_frustum_planes[0].x = vp_matrix._14 + vp_matrix._11;
        m_frustum_planes[0].y = vp_matrix._24 + vp_matrix._21;
        m_frustum_planes[0].z = vp_matrix._34 + vp_matrix._31;
        m_frustum_planes[0].w = vp_matrix._44 + vp_matrix._41;

        // Right plane
        m_frustum_planes[1].x = vp_matrix._14 - vp_matrix._11;
        m_frustum_planes[1].y = vp_matrix._24 - vp_matrix._21;
        m_frustum_planes[1].z = vp_matrix._34 - vp_matrix._31;
        m_frustum_planes[1].w = vp_matrix._44 - vp_matrix._41;

        // Top plane
        m_frustum_planes[2].x = vp_matrix._14 - vp_matrix._12;
        m_frustum_planes[2].y = vp_matrix._24 - vp_matrix._22;
        m_frustum_planes[2].z = vp_matrix._34 - vp_matrix._32;
        m_frustum_planes[2].w = vp_matrix._44 - vp_matrix._42;

        // Bottom plane
        m_frustum_planes[3].x = vp_matrix._14 + vp_matrix._12;
        m_frustum_planes[3].y = vp_matrix._24 + vp_matrix._22;
        m_frustum_planes[3].z = vp_matrix._34 + vp_matrix._32;
        m_frustum_planes[3].w = vp_matrix._44 + vp_matrix._42;

        // Near plane
        m_frustum_planes[4].x = vp_matrix._13;
        m_frustum_planes[4].y = vp_matrix._23;
        m_frustum_planes[4].z = vp_matrix._33;
        m_frustum_planes[4].w = vp_matrix._43;

        // Far plane
        m_frustum_planes[5].x = vp_matrix._14 - vp_matrix._13;
        m_frustum_planes[5].y = vp_matrix._24 - vp_matrix._23;
        m_frustum_planes[5].z = vp_matrix._34 - vp_matrix._33;
        m_frustum_planes[5].w = vp_matrix._44 - vp_matrix._43;

        // 归一化平面
        for (int i = 0; i < 6; ++i)
        {
            XMVECTOR plane = XMLoadFloat4(&m_frustum_planes[i]);
            plane = XMPlaneNormalize(plane);
            XMStoreFloat4(&m_frustum_planes[i], plane);
        }

        m_frustum_dirty = false;
    }

} // namespace Dolas 