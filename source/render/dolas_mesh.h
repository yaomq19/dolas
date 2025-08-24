#ifndef DOLAS_MESH_H
#define DOLAS_MESH_H

#include <string>
#include <vector>
#include <DirectXMath.h>
#include "common/dolas_hash.h"

namespace Dolas
{
    enum class DolasRenderPrimitiveType: UInt
    {
        _TRIANGLE_LIST
    };

	enum class DolasInputLayoutType : UInt
	{
		POS_3_UV_2,
        POS_3_UV_2_NORM_3
	};

    class Mesh
    {
        friend class MeshManager;
    public:
        Mesh();
        ~Mesh();

        MeshID m_file_id;
        
        // RenderPrimitive needs these
        DolasRenderPrimitiveType m_render_primitive_type;
        DolasInputLayoutType m_input_layout_type;
        std::vector<unsigned int> m_indices;
		std::vector<float> m_final_vertices;

        // temp data
		std::vector<DirectX::XMFLOAT3> m_vertices;
		std::vector<DirectX::XMFLOAT2> m_uvs;
		std::vector<DirectX::XMFLOAT3> m_normals;
		std::vector<DirectX::XMFLOAT3> m_tangents;
		std::vector<DirectX::XMFLOAT3> m_bitangents;

        RenderPrimitiveID GetRenderPrimitiveID() const { return m_render_primitive_id; }
    protected:
        RenderPrimitiveID m_render_primitive_id;
    }; // class Mesh
} // namespace Dolas

#endif // DOLAS_MESH_H