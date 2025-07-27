#ifndef DOLAS_MESH_MANAGER_H
#define DOLAS_MESH_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "base/dolas_base.h"
#include "common/dolas_hash.h"
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

        MeshID CreateMesh(const std::string& mesh_file_name);
        Mesh* GetMesh(MeshID mesh_id);
    private:
        std::unordered_map<MeshID, Mesh*> m_meshes;
    };// class MeshManager
}// namespace Dolas
#endif // DOLAS_MESH_MANAGER_H
