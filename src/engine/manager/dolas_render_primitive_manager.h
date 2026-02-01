#ifndef DOLAS_RENDER_PRIMITIVE_MANAGER_H
#define DOLAS_RENDER_PRIMITIVE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "core/dolas_rhi_common.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    class RenderPrimitive;

	enum BaseGeometryType : UInt
	{
		BaseGeometryType_TRIANGLE,
		BaseGeometryType_QUAD,
		BaseGeometryType_CUBE,
		BaseGeometryType_SPHERE,
		BaseGeometryType_CYLINDER,
		BaseGeometryType_CONE,
	};

    class RenderPrimitiveManager
    {
    public:
        RenderPrimitiveManager();
        ~RenderPrimitiveManager();

        Bool Initialize();
        Bool Clear();

        // Create RenderPrimitive
		// @param id: Specifies the ID of the RenderPrimitive to be created
		// @param render_primitive_type: Specifies the topology of the RenderPrimitive to create
		// @param input_layout_type: Specifies the input layout of the RenderPrimitive to be created
		// @param vertices: Providing vertex data
		// @param indices: Providing index data
        Bool CreateRenderPrimitive(
            RenderPrimitiveID id,
            const PrimitiveTopology& render_primitive_type,
            const InputLayoutType& input_layout_type,
            const std::vector<std::vector<Float>>& vertices,
            const std::vector<UInt>& indices);

		RenderPrimitiveID GetGeometryRenderPrimitiveID(BaseGeometryType geometry_type);
        RenderPrimitive* GetRenderPrimitiveByID(RenderPrimitiveID render_primitive_id) const;

        // 从 .mesh 文件创建 RenderPrimitive，返回对应的 RenderPrimitiveID
        RenderPrimitiveID CreateRenderPrimitiveFromMeshFile(const std::string& mesh_file_name);
    private:
		Bool InitializeSphereGeometry();
		Bool InitializeQuadGeometry();
		Bool InitializeCylinderGeometry();
		Bool InitializeCubeGeometry();

		Bool GenerateSphereRawData(UInt segments, std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices);
		Bool GenerateQuadRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices);
		Bool GenerateCylinderRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices);
		Bool GenerateCubeRawData(std::vector<std::vector<Float>>& vertices_data, std::vector<UInt>& indices);

        RenderPrimitive* BuildFromRawData(
			const PrimitiveTopology& render_primitive_type,
			const InputLayoutType& input_layout_type,
			const std::vector<std::vector<Float>>& vertices,
			const std::vector<UInt>& indices);

        std::unordered_map<RenderPrimitiveID, RenderPrimitive*> m_render_primitives;
        std::unordered_map<BaseGeometryType, RenderPrimitiveID> m_base_geometries;
    }; // class RenderPrimitiveManager
}

#endif // DOLAS_RENDER_PRIMITIVE_MANAGER_H


