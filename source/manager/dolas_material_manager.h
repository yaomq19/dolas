#ifndef DOLAS_MATERIAL_MANAGER_H
#define DOLAS_MATERIAL_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>

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

        Material* GetOrCreateMaterial(const std::string& file_name);
    private:
        Material* CreateMaterial(const std::string& file_name);
        std::unordered_map<std::string, Material*> m_materials;
    };// class MaterialManager
}// namespace Dolas
#endif // DOLAS_MATERIAL_MANAGER_H