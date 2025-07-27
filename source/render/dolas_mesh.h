#ifndef DOLAS_MESH_H
#define DOLAS_MESH_H

#include <string>
#include <vector>
#include <DirectXMath.h>
#include "common/dolas_hash.h"

namespace Dolas
{
    class Mesh
    {
        friend class MeshManager;
    public:
        Mesh();
        ~Mesh();

        MeshID m_file_id;
        std::vector<DirectX::XMFLOAT3> m_vertices;
        std::vector<DirectX::XMFLOAT2> m_uvs;
        std::vector<DirectX::XMFLOAT3> m_normals;
        std::vector<DirectX::XMFLOAT3> m_tangents;
        std::vector<DirectX::XMFLOAT3> m_bitangents;
        std::vector<unsigned int> m_indices;
            
        std::vector<float> m_final_vertices;
    }; // class Mesh
} // namespace Dolas

#endif // DOLAS_MESH_H