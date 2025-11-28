#ifndef DOLAS_GEOMETRY_H
#define DOLAS_GEOMETRY_H

#include "base/dolas_base.h"
#include "common/dolas_hash.h"

enum BaseGeometryType : UInt
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
    protected:
		Bool InitializeSphereGeometry();
        Bool InitializeQuadGeometry();

        Bool GenerateSphereRawData(UInt segments, std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices);
        Bool GenerateQuadRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices);
    protected:
        std::unordered_map<BaseGeometryType, RenderPrimitiveID> m_geometries;
    };// class GeometryManager
} // namespace Dolas
#endif // DOLAS_GEOMETRY_MANAGER_H