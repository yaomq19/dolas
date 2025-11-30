#ifndef DOLAS_DEBUG_DRAW_MANAGER_H
#define DOLAS_DEBUG_DRAW_MANAGER_H
#include <vector>
#include "core/dolas_math.h"
#include "common/dolas_color.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    struct DebugDrawObject
    {
        RenderPrimitiveID m_render_primitive_id;
        Color m_color;
        Float m_life_time;
    };

    class DebugDrawManager
    {
    public:
        DebugDrawManager();
        ~DebugDrawManager();

        bool Initialize();
        bool Clear();

        void Render();
        void Tick(Float delta_time);

        void AddCylinder(const Vector3& center, const Float radius, const Float height, const Quaternion& rotation, const Color& color, Float life_time = -1.0f);
    protected:
        std::vector<DebugDrawObject> m_render_objects;
    };
}// namespace Dolas

#endif // DOLAS_DEBUG_DRAW_MANAGER_H