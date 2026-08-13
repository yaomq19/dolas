#ifndef DOLAS_HASH_H
#define DOLAS_HASH_H

#include <string>
#include <unordered_map>
#include "dolas_base.h"

namespace Dolas
{
    #define STRING_ID(x) Dolas::HashConverter::StringHash(#x)
    #define ID_TO_STRING(x) Dolas::HashConverter::GetString(x)
    
    class HashConverter
    {
    public:
        static UInt StringHash(const std::string& str);
        static std::string GetString(UInt hash);
        static Bool HasString(UInt hash);
        static void ClearRegistry();
    private:
        /// Internal mapping table for hash-to-string reverse lookup
        /// Using function-local static to avoid static initialization order fiasco
        static std::unordered_map<UInt, std::string>& GetHashToStringMap();
    };
    
}

#endif