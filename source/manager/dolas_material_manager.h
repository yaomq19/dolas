#ifndef DOLAS_MATERIAL_MANAGER_H
#define DOLAS_MATERIAL_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "base/dolas_base.h"
#include "common/dolas_hash.h"
namespace Dolas
{
    class Material;
    class MaterialManager
    {
    public:
        MaterialManager();
        ~MaterialManager();
        bool Initialize();
        bool Clear();

        MaterialID CreateMaterial(const std::string& file_name);
        Material* GetMaterial(MaterialID material_id);
    private:
        std::unordered_map<MaterialID, Material*> m_materials;
    };// class MaterialManager
}// namespace Dolas
#endif // DOLAS_MATERIAL_MANAGER_H