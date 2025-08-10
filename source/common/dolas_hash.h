#ifndef DOLAS_HASH_H
#define DOLAS_HASH_H

#include "base/dolas_base.h"
#include <string>
#include <unordered_map>

// Macro for compile-time string to hash conversion
// Usage: UInt id = STRING_ID(ResourceName);
#define STRING_ID(x) Dolas::HashConverter::StringHash(#x)
#define ID_TO_STRING(x) Dolas::HashConverter::GetString(x)

typedef UInt StringID;
typedef UInt FileID;
typedef UInt MaterialID;
typedef UInt MeshID;
typedef UInt TextureID;
typedef UInt ShaderID;
typedef UInt RenderEntityID;
typedef UInt RenderResourceID;

#define STRING_ID_EMPTY 0
#define FILE_ID_EMPTY 0
#define MATERIAL_ID_EMPTY 0
#define MESH_ID_EMPTY 0
#define TEXTURE_ID_EMPTY 0
#define SHADER_ID_EMPTY 0
#define RENDER_ENTITY_ID_EMPTY 0
#define RENDER_RESOURCE_ID_EMPTY 0

namespace Dolas
{
    /**
     * @brief Hash utility class for string-to-hash conversion and reverse lookup
     * 
     * This class provides fast string hashing using the FNV-1a algorithm,
     * which is commonly used in game engines for resource identification.
     * It also provides reverse lookup functionality for debugging purposes.
     */
    class HashConverter
    {
    public:
        /**
         * @brief Convert a string to a 32-bit hash value using FNV-1a algorithm
         * 
         * @param str The input string to hash
         * @return UInt The 32-bit hash value
         * 
         * @note In debug builds, strings are automatically registered for reverse lookup
         * @note FNV-1a is fast, has good distribution, and is widely used in game engines
         */
        static UInt StringHash(const std::string& str);
        
        /**
         * @brief Manually register a string for reverse lookup
         * 
         * @param str The string to register in the lookup table
         * 
         * @note This function calculates the hash and stores the mapping
         * @note Useful for registering strings that need to be looked up later
         */
        /*static void RegisterString(const std::string& str);*/
        
        /**
         * @brief Get the original string from a hash value
         * 
         * @param hash The hash value to look up
         * @return std::string The original string if found, or "Unknown[hash]" if not found
         * 
         * @note This function is primarily for debugging purposes
         * @note Returns a formatted string with the hash value if the original string is not found
         */
        static std::string GetString(UInt hash);
        
        /**
         * @brief Check if a hash value has a registered string
         * 
         * @param hash The hash value to check
         * @return Bool True if the hash has a corresponding string, false otherwise
         * 
         * @note Use this to avoid string allocation when you only need to check existence
         */
        static Bool HasString(UInt hash);
        
        /**
         * @brief Clear all registered strings from the lookup table
         * 
         * @note Useful for memory management or when switching between different sets of resources
         * @note This will make all previous reverse lookups return "Unknown[hash]" format
         */
        static void ClearRegistry();
        
    private:
        /// Internal mapping table for hash-to-string reverse lookup
        static std::unordered_map<UInt, std::string> s_hashToString;
    };
    
}

#endif