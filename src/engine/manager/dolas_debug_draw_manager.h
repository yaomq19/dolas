#ifndef DOLAS_DEBUG_DRAW_MANAGER_H
#define DOLAS_DEBUG_DRAW_MANAGER_H
#include <vector>
#include "core/dolas_math.h"
#include "common/dolas_color.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    const static Float k_instant_life_time = -1.0f;
    struct DebugDrawObject
    {
		RenderPrimitiveID m_render_primitive_id = RENDER_PRIMITIVE_ID_EMPTY;
		Pose m_pose;
        Color m_color = Color::WHITE;
		Float m_life_time = -1.0f; // -1 means split-second life time
    };

    class DebugDrawManager
    {
    public:
        DebugDrawManager();
        ~DebugDrawManager();

        bool Initialize();
        bool Clear();
        void TickPostRender(Float delta_time);
		const std::vector<DebugDrawObject>& GetDebugObjects() const { return m_render_objects; }
        void AddCylinder(const Vector3& center, const Float radius, const Float height, const Quaternion& rotation, const Color& color, Float life_time = k_instant_life_time);
        void AddSphere(const Vector3& center, const Float radius, const Color& color, Float life_time = k_instant_life_time);
    protected:
        std::vector<DebugDrawObject> m_render_objects;
    };
}// namespace Dolas

#endif // DOLAS_DEBUG_DRAW_MANAGER_H