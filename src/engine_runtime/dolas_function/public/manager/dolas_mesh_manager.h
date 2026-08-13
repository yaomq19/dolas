#ifndef DOLAS_MESH_MANAGER_H
#define DOLAS_MESH_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "dolas_base.h"
#include "dolas_hash.h"

namespace Dolas
{
    // MeshManager 已废弃，仅保留空壳以兼容旧的包含路径
    class MeshManager
    {
    public:
        MeshManager() = default;
        ~MeshManager() = default;

        bool Initialize() { return true; }
        bool Clear() { return true; }
    };
}// namespace Dolas
#endif // DOLAS_MESH_MANAGER_H