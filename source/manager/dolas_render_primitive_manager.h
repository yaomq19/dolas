#ifndef DOLAS_RENDER_PRIMITIVE_MANAGER_H
#define DOLAS_RENDER_PRIMITIVE_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>

#include "common/dolas_hash.h"
#include "manager/dolas_mesh_manager.h"
#include "render/dolas_mesh.h"
namespace Dolas
{
    class RenderPrimitive;

    class RenderPrimitiveManager
    {
    public:
        RenderPrimitiveManager();
        ~RenderPrimitiveManager();

        bool Initialize();
        bool Clear();

        // Create RenderPrimitive
		// @param id: Specifies the ID of the RenderPrimitive to be created
		// @param render_primitive_type: Specifies the topology of the RenderPrimitive to create
		// @param input_layout_type: Specifies the input layout of the RenderPrimitive to be created
		// @param vertices: Providing vertex data
		// @param indices: Providing index data
        Bool CreateRenderPrimitive(
            RenderPrimitiveID id,
            const RenderPrimitiveTopology& render_primitive_type,
            const InputLayoutType& input_layout_type,
            const std::vector<Float>& vertices,
            const std::vector<UInt>& indices);

        RenderPrimitive* GetRenderPrimitiveByID(RenderPrimitiveID render_primitive_id);
    private:

        RenderPrimitive* BuildFromRawData(
			RenderPrimitiveID id,
			const RenderPrimitiveTopology& render_primitive_type,
			const InputLayoutType& input_layout_type,
			const std::vector<float>& vertices,
			const std::vector<unsigned int>& indices);

        std::unordered_map<RenderPrimitiveID, RenderPrimitive*> m_render_primitives;
    }; // class RenderPrimitiveManager
}

#endif // DOLAS_RENDER_PRIMITIVE_MANAGER_H


