#include "manager/dolas_asset_manager.h"
#include "base/dolas_base.h"
#include <fstream>
#include <iostream>
namespace Dolas
{
    AssetManager::AssetManager()
    {
    }

    AssetManager::~AssetManager()
    {
    }

    Bool AssetManager::Initialize()
    {
        return true;
    }

    Bool AssetManager::Clear()
    {
        return true;
    }

    Bool AssetManager::LoadJsonFile(const std::string& file_path, json& json_data)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            return false;
        }
        file >> json_data;
        file.close();
        return true;
    }
}