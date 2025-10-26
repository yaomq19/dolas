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
		// @param id: ָ��Ҫ������ RenderPrimitive �� ID
		// @param render_primitive_type: ָ��Ҫ������ RenderPrimitive ������
		// @param input_layout_type: ָ��Ҫ������ RenderPrimitive �����벼��
		// @param vertices: �ṩ��������
		// @param indices: �ṩ��������
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


