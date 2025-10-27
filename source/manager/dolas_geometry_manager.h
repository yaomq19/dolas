#ifndef DOLAS_GEOMETRY_H
#define DOLAS_GEOMETRY_H

#include "base/dolas_base.h"
#include "common/dolas_hash.h"

enum class BaseGeometryType : UInt
{
    _TRIANGLE,
    _QUAD,
    _CUBE,
    _SPHERE,
    _CYLINDER,
    _CONE,
};

namespace Dolas
{
    class GeometryManager
    {
    public:
        GeometryManager();
        ~GeometryManager();
        bool Initialize();
        bool Clear();
        RenderPrimitiveID GetGeometryRenderPrimitiveID(BaseGeometryType geometry_type);
    private:
        std::unordered_map<BaseGeometryType, RenderPrimitiveID> m_geometries;

        Bool generateSphereGeometry(UInt segments, std::vector<Float>& vertices, std::vector<UInt>& indices);
    };// class GeometryManager
} // namespace Dolas
#endif // DOLAS_GEOMETRY_MANAGER_H