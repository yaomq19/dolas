#include "render/dolas_render_camera.h"
#include <iostream>
#include <cmath>

namespace Dolas
{
    using namespace DirectX;
    /* Render Camera */
    // 顺序：构造 -> 析构 -> Getters -> Setters -> 受保护更新函数
    RenderCamera::RenderCamera()
    :   m_camera_perspective_type(CameraPerspectiveType::PERSPECTIVE),
        m_position(0.0f, 0.0f, 0.0f),
        m_forward(0.0f, 0.0f, 1.0f),
        m_up(0.0f, 1.0f, 0.0f),
        m_near_plane(0.1f),
        m_far_plane(100.0f)
    {
        UpdateViewMatrix();
    }

    RenderCamera::RenderCamera(CameraPerspectiveType camera_perspective_type)
    :   m_camera_perspective_type(camera_perspective_type),
        m_position(0.0f, 0.0f, 0.0f),
        m_forward(0.0f, 0.0f, 1.0f),
        m_up(0.0f, 1.0f, 0.0f),
        m_near_plane(0.1f),
        m_far_plane(100.0f)
    {
        UpdateViewMatrix();
    }

    RenderCamera::RenderCamera(CameraPerspectiveType camera_perspective_type, const Vector3& position)
    :   m_camera_perspective_type(camera_perspective_type),
        m_position(position),
        m_forward(0.0f, 0.0f, 1.0f),
        m_up(0.0f, 1.0f, 0.0f),
        m_near_plane(0.1f),
        m_far_plane(100.0f)
    {
        UpdateViewMatrix();
    }

    RenderCamera::RenderCamera(CameraPerspectiveType camera_perspective_type, const Vector3& position, const Vector3& forward, const Vector3& up)
    :   m_camera_perspective_type(camera_perspective_type),
        m_position(position),
        m_forward(forward),
        m_up(up),
        m_near_plane(0.1f),
        m_far_plane(100.0f) 
    {
        UpdateViewMatrix();
    }

    RenderCamera::RenderCamera(CameraPerspectiveType camera_perspective_type, const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane)
    :   m_camera_perspective_type(camera_perspective_type),
        m_position(position),
        m_forward(forward),
        m_up(up),
        m_near_plane(near_plane),
        m_far_plane(far_plane)
    {
        UpdateViewMatrix();
    }

    RenderCamera::~RenderCamera()
    {

    }

    CameraPerspectiveType RenderCamera::GetCameraPerspectiveType() const
    {
        return m_camera_perspective_type;
    }

    Matrix4x4 RenderCamera::GetProjectionMatrix() const
    {
        return m_projection_matrix;
    }

    Matrix4x4 RenderCamera::GetViewMatrix() const
    {
        return m_view_matrix;
    }

    Vector3 RenderCamera::GetPosition() const
    {
        return m_position;
    }

    Vector3 RenderCamera::GetForward() const
    {
        return m_forward;
    }

    Vector3 RenderCamera::GetUp() const
    {
        return m_up;
    }

    Float RenderCamera::GetNearPlane() const
    {
        return m_near_plane;
    }

    Float RenderCamera::GetFarPlane() const
    {
        return m_far_plane;
    }

    void RenderCamera::SetPosition(const Vector3& position)
    {
        m_position = position;
        UpdateViewMatrix();
    }

    void RenderCamera::SetForwardAndUp(const Vector3& forward, const Vector3& up)
    {
        m_forward = forward.Normalized();
        m_up = up.Normalized();
        CorrectUpVector();
        UpdateViewMatrix();
    }

    void RenderCamera::SetNearPlane(Float near_plane)
    {
        m_near_plane = near_plane;
        UpdateProjectionMatrix();
    }

    void RenderCamera::SetFarPlane(Float far_plane)
    {
        m_far_plane = far_plane;
        UpdateProjectionMatrix();
    }

    void RenderCamera::CorrectUpVector()
    {
        Vector3 right = m_forward.Cross(m_up).Normalized();
        m_up = right.Cross(m_forward).Normalized();
    }

    void RenderCamera::TestRotate(Float delta_time)
    {
		static Float time = 0.0f;
        SetPosition(Vector3(m_position.x + std::sin(time + delta_time), m_position.y, m_position.z));
    }

    void RenderCamera::ProcessMouseInput(Float mouse_delta_x, Float mouse_delta_y, Float sensitivity)
    {
        // 将鼠标移动转换为角度变化（弧度）
        Float yaw_delta = -mouse_delta_x * sensitivity * 0.01f;   // 水平旋转（偏航角）
        Float pitch_delta = -mouse_delta_y * sensitivity * 0.01f; // 垂直旋转（俯仰角），注意Y轴反转
        
        // 获取右向量
        
        // 绕世界Y轴旋转（偏航）
        Matrix3x3 yaw_rotation = MathUtil::Rotate(Vector3::Z(), yaw_delta);
        m_forward = yaw_rotation * m_forward;
        m_up = yaw_rotation * m_up;
        m_forward.Normalize();
        m_up.Normalize();

        // 绕右向量旋转（俯仰）
        Matrix3x3 pitch_rotation = MathUtil::Rotate(GetRightVector(), pitch_delta);
        m_forward = pitch_rotation * m_forward;
        m_up = pitch_rotation * m_up;
        
        // 标准化向量
        m_forward = m_forward.Normalized();
        m_up = m_up.Normalized();
        
        // 确保向量正交
        CorrectUpVector();
        
        // 更新视图矩阵
        UpdateViewMatrix();
    }

    void RenderCamera::ProcessKeyboardInput(bool move_forward, bool move_backward, bool move_left, bool move_right, 
                                          bool move_up, bool move_down, Float delta_time, Float move_speed)
    {
        Vector3 movement(0.0f, 0.0f, 0.0f);
        Vector3 right = GetRightVector();
        
        // 前后移动
        if (move_forward)
        {
            movement += m_forward * move_speed * delta_time;
        }
        if (move_backward)
        {
            movement -= m_forward * move_speed * delta_time;
        }
        
        // 左右移动
        if (move_left)
        {
            movement -= right * move_speed * delta_time;
        }
        if (move_right)
        {
            movement += right * move_speed * delta_time;
        }
        
        // 上下移动
        if (move_up)
        {
            movement += Vector3::Z() * move_speed * delta_time;
        }
        if (move_down)
        {
            movement -= Vector3::Z() * move_speed * delta_time;
        }
        
        // 应用移动
        if (movement.Length() > 0.0f)
        {
            SetPosition(m_position + movement);
        }
    }

    Bool RenderCamera::BuildFromFile(const std::string& file_name)
    {

        return true;
    }

    Vector3 RenderCamera::GetRightVector() const
    {
        return m_forward.Cross(m_up).Normalized();
    }

    void RenderCamera::UpdateViewMatrix()
    {
        Matrix4x4 move_to_origin_matrix = Matrix4x4::Identity();
        move_to_origin_matrix.data[0][3] = -m_position.x;
        move_to_origin_matrix.data[1][3] = -m_position.y;
        move_to_origin_matrix.data[2][3] = -m_position.z;

        Vector3 n_axis = m_forward.Cross(Vector3::NegativeZ()).Normalized();
        Float theta = std::acos(m_forward.Dot(Vector3::NegativeZ()));
        Matrix3x3 forward_to_negative_z_matrix = MathUtil::Rotate(n_axis, theta);

        Vector3 up_prime = forward_to_negative_z_matrix * m_up;
        n_axis = up_prime.Cross(Vector3::Y()).Normalized();
        theta = std::acos(up_prime.Dot(Vector3::Y()));
        Matrix3x3 up_to_y_matrix = MathUtil::Rotate(n_axis, theta);

        Matrix3x3 rotate_camera_matrix = up_to_y_matrix * forward_to_negative_z_matrix;

        m_view_matrix = rotate_camera_matrix.ExpandToMatrix4x4() * move_to_origin_matrix;
    }

    /* Render Camera Perspective */
    RenderCameraPerspective::RenderCameraPerspective()
    :   RenderCamera(CameraPerspectiveType::PERSPECTIVE),
        m_aspect_ratio(1.0f),
        m_fov(0.78539816339f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraPerspective::RenderCameraPerspective(const Vector3& position)
    :   RenderCamera(CameraPerspectiveType::PERSPECTIVE, position),
        m_aspect_ratio(1.0f),
        m_fov(0.78539816339f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraPerspective::RenderCameraPerspective(const Vector3& position, const Vector3& forward, const Vector3& up)
    :   RenderCamera(CameraPerspectiveType::PERSPECTIVE, position, forward, up),
        m_aspect_ratio(1.0f),
        m_fov(0.78539816339f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraPerspective::RenderCameraPerspective(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane)
    :   RenderCamera(CameraPerspectiveType::PERSPECTIVE, position, forward, up, near_plane, far_plane),
        m_aspect_ratio(1.0f),
        m_fov(0.78539816339f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraPerspective::RenderCameraPerspective(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane, Float fov, Float aspect_ratio)
    :   RenderCamera(CameraPerspectiveType::PERSPECTIVE, position, forward, up, near_plane, far_plane),
        m_aspect_ratio(aspect_ratio),
        m_fov(fov)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraPerspective::~RenderCameraPerspective()
    {
    }

    void RenderCameraPerspective::SetAspectRatio(Float aspect_ratio)
    {
        m_aspect_ratio = aspect_ratio;
        UpdateProjectionMatrix();
    }

    Float RenderCameraPerspective::GetAspectRatio() const
    {
        return m_aspect_ratio;
    }

    void RenderCameraPerspective::SetFov(Float fov)
    {
        m_fov = fov;
        UpdateProjectionMatrix();
    }

    Float RenderCameraPerspective::GetFov() const
    {
        return m_fov;
    }

    void RenderCameraPerspective::UpdateProjectionMatrix()
    {
        m_projection_matrix = Matrix4x4::Perspective(
            MathUtil::DegreesToRadians(m_fov),
            m_aspect_ratio,
            -m_far_plane,
            -m_near_plane);
    }

    /* Render Camera Orthographic */
    RenderCameraOrthographic::RenderCameraOrthographic()
    :   RenderCamera(CameraPerspectiveType::ORTHOGRAPHIC),
        m_window_width(1.0f),
        m_window_height(1.0f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraOrthographic::RenderCameraOrthographic(const Vector3& position)
    :   RenderCamera(CameraPerspectiveType::ORTHOGRAPHIC, position),
        m_window_width(1.0f),
        m_window_height(1.0f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraOrthographic::RenderCameraOrthographic(const Vector3& position, const Vector3& forward, const Vector3& up)
    :   RenderCamera(CameraPerspectiveType::ORTHOGRAPHIC, position, forward, up),
        m_window_width(1.0f),
        m_window_height(1.0f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraOrthographic::RenderCameraOrthographic(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane)
    :   RenderCamera(CameraPerspectiveType::ORTHOGRAPHIC, position, forward, up, near_plane, far_plane),
        m_window_width(1.0f),
        m_window_height(1.0f)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraOrthographic::RenderCameraOrthographic(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane, Float window_width, Float window_height)
    :   RenderCamera(CameraPerspectiveType::ORTHOGRAPHIC, position, forward, up, near_plane, far_plane),
        m_window_width(window_width),
        m_window_height(window_height)
    {
        UpdateProjectionMatrix();
    }

    RenderCameraOrthographic::~RenderCameraOrthographic()
    {
    }

    void RenderCameraOrthographic::SetWindowWidth(Float window_width)
    {
        m_window_width = window_width;
        UpdateProjectionMatrix();
    }

    Float RenderCameraOrthographic::GetWindowWidth() const
    {
        return m_window_width;
    }

    void RenderCameraOrthographic::SetWindowHeight(Float window_height)
    {
        m_window_height = window_height;
        UpdateProjectionMatrix();
    }

    Float RenderCameraOrthographic::GetWindowHeight() const
    {
        return m_window_height;
    }

    void RenderCameraOrthographic::UpdateProjectionMatrix()
    {
        m_projection_matrix = Matrix4x4::Orthographic(
            -m_window_width / 2.0f,
            m_window_width / 2.0f,
            m_window_height / 2.0f,
            -m_window_height / 2.0f,
            -m_far_plane,
            -m_near_plane);
    }

} // namespace Dolas 