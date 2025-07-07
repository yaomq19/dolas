#ifndef DOLAS_MESH_H
#define DOLAS_MESH_H

#include <string>
#include <vector>
#include <DirectXMath.h>

namespace Dolas
{
    class Mesh
    {
        friend class MeshManager;
    public:
        Mesh();
        ~Mesh();

        std::string m_file_path;
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