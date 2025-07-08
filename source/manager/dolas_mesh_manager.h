#ifndef DOLAS_MESH_MANAGER_H
#define DOLAS_MESH_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>

namespace Dolas
{
    class Mesh;
    class MeshManager
    {
    public:
        MeshManager();
        ~MeshManager();
        bool Initialize();
        bool Clear();

        Mesh* GetOrCreateMesh(const std::string& file_name);
    private:
        Mesh* CreateMesh(const std::string& file_name);
        std::unordered_map<std::string, Mesh*> m_meshes;
    };// class MeshManager
}// namespace Dolas
#endif // DOLAS_MESH_MANAGER_H
