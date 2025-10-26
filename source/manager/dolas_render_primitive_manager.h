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
		// @param id: 指定要创建的 RenderPrimitive 的 ID
		// @param render_primitive_type: 指定要创建的 RenderPrimitive 的拓扑
		// @param input_layout_type: 指定要创建的 RenderPrimitive 的输入布局
		// @param vertices: 提供顶点数据
		// @param indices: 提供索引数据
        Bool CreateRenderPrimitive(
            RenderPrimitiveID id,
            const DolasRenderPrimitiveType& render_primitive_type,
            const DolasInputLayoutType& input_layout_type,
            const std::vector<Float>& vertices,
            const std::vector<UInt>& indices);

        RenderPrimitive* GetRenderPrimitiveByID(RenderPrimitiveID render_primitive_id);
    private:

        RenderPrimitive* BuildFromRawData(
			RenderPrimitiveID id,
			const DolasRenderPrimitiveType& render_primitive_type,
			const DolasInputLayoutType& input_layout_type,
			const std::vector<float>& vertices,
			const std::vector<unsigned int>& indices);

        std::unordered_map<RenderPrimitiveID, RenderPrimitive*> m_render_primitives;
    }; // class RenderPrimitiveManager
}

#endif // DOLAS_RENDER_PRIMITIVE_MANAGER_H


