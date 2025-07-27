#ifndef DOLAS_ASSET_MANAGER_H
#define DOLAS_ASSET_MANAGER_H

#include <string>
#include "base/dolas_base.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Dolas
{
    class AssetManager
    {
    public:
        AssetManager();
        ~AssetManager();

        Bool Initialize();
        Bool Clear();

        Bool LoadJsonFile(const std::string& file_path, json& json_data);

    };
}
#endif // DOLAS_ASSET_MANAGER_H