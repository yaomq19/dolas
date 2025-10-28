#ifndef DOLAS_RENDER_CAMERA_H
#define DOLAS_RENDER_CAMERA_H

#include <DirectXMath.h>
#include <string>
#include "common/dolas_hash.h"
#include "core/dolas_math.h"
namespace Dolas
{
    enum class CameraPerspectiveType
    {
        PERSPECTIVE,
        ORTHOGRAPHIC
    };

    class RenderCamera
    {
        friend class RenderCameraManager;
    public:
        RenderCamera();
        RenderCamera(CameraPerspectiveType camera_perspective_type);
        RenderCamera(CameraPerspectiveType camera_perspective_type, const Vector3& position);
        RenderCamera(CameraPerspectiveType camera_perspective_type, const Vector3& position, const Vector3& forward, const Vector3& up);
        RenderCamera(CameraPerspectiveType camera_perspective_type, const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane);
        ~RenderCamera();
        
		void ProcessWheelDelta(Float wheel_delta);
        CameraPerspectiveType GetCameraPerspectiveType() const;
        Matrix4x4 GetProjectionMatrix() const;
        Matrix4x4 GetViewMatrix() const;
        Vector3 GetPosition() const;
        Vector3 GetForward() const;
        Vector3 GetUp() const;
        Float GetNearPlane() const;
        Float GetFarPlane() const;
        
        void SetPosition(const Vector3& position);
        void SetForwardAndUp(const Vector3& forward, const Vector3& up);
        void SetNearPlane(Float near_plane);
        void SetFarPlane(Float far_plane);

        void TestRotate(Float delta_time);
        
        // 相机控制函数
        void ProcessMouseInput(Float mouse_delta_x, Float mouse_delta_y);
        void ProcessKeyboardInput(bool move_forward, bool move_backward, bool move_left, bool move_right, 
                                bool move_up, bool move_down, Float delta_time);

        virtual void BuildFromAsset(class CameraAsset* camera_asset) = 0;

        void PrintDebugInfo();
        protected:
        void CorrectUpVector();
        void UpdateViewMatrix();
        virtual void UpdateProjectionMatrix() = 0;
        
        // 相机控制辅助函数
        Vector3 GetRightVector() const;

        CameraPerspectiveType m_camera_perspective_type;
        Vector3 m_position;
        Vector3 m_forward;
        Vector3 m_up;
        Float m_near_plane;
        Float m_far_plane;

        Matrix4x4 m_projection_matrix;
        Matrix4x4 m_view_matrix;

        Float m_move_speed{ 0.03f };
    }; // class RenderCamera

    class RenderCameraPerspective : public RenderCamera
    {
    public:
        RenderCameraPerspective();
        RenderCameraPerspective(const Vector3& position);
        RenderCameraPerspective(const Vector3& position, const Vector3& forward, const Vector3& up);
        RenderCameraPerspective(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane);
        RenderCameraPerspective(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane, Float fov, Float aspect_ratio);
        ~RenderCameraPerspective();

        void SetAspectRatio(Float aspect_ratio);
        Float GetAspectRatio() const;

        void SetFov(Float fov);
        Float GetFov() const;

        virtual void UpdateProjectionMatrix() override;
        virtual void BuildFromAsset(class CameraAsset* camera_asset) override;

        protected:
        Float m_aspect_ratio;
        Float m_fov; // in degree
    };

    class RenderCameraOrthographic : public RenderCamera
    {
    public:
        RenderCameraOrthographic();
        RenderCameraOrthographic(const Vector3& position);
        RenderCameraOrthographic(const Vector3& position, const Vector3& forward, const Vector3& up);
        RenderCameraOrthographic(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane);
        RenderCameraOrthographic(const Vector3& position, const Vector3& forward, const Vector3& up, Float near_plane, Float far_plane, Float window_width, Float window_height);
        ~RenderCameraOrthographic();

        void SetWindowWidth(Float window_width);
        Float GetWindowWidth() const;

        void SetWindowHeight(Float window_height);
        Float GetWindowHeight() const;

        virtual void UpdateProjectionMatrix() override;
		virtual void BuildFromAsset(class CameraAsset* camera_asset) override;

        protected:
        Float m_window_width;
        Float m_window_height;
    };
} // namespace Dolas

#endif // DOLAS_RENDER_CAMERA_H 