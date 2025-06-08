#ifndef DOLA_RENDER_ENTITY_H
#define DOLA_RENDER_ENTITY_H

#include <d3d11.h>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include "dolas_rhi.h"

namespace Dolas
{
    class Mesh
    {
    public:
        Mesh();
        ~Mesh();

        void Load(const std::string& file_path);

        std::string m_file_path;
        std::vector<DirectX::XMFLOAT3> m_vertices;
        std::vector<DirectX::XMFLOAT2> m_uvs;
        std::vector<DirectX::XMFLOAT3> m_normals;
        std::vector<DirectX::XMFLOAT3> m_tangents;
        std::vector<DirectX::XMFLOAT3> m_bitangents;
        
    };

    class Material
    {
    public:
        Material();
        ~Material();

        void Load(const std::string& file_path);

        std::string m_file_path;
        ID3D11VertexShader* m_vertex_shader = nullptr;
        ID3D11PixelShader* m_pixel_shader = nullptr;
        std::vector<std::pair<int, std::string>> m_textures;
    };

    class MeshRenderEntity
    {
    public:
        MeshRenderEntity();
        ~MeshRenderEntity();
        void Draw(DolasRHI* rhi);
        void Load(const std::string& file_path);
    private:
        std::string m_file_path;
        Mesh m_mesh;
        Material m_material;
    };
} // namespace Dolas

#endif // DOLA_RENDER_ENTITY_H