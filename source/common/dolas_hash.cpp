#include "common/dolas_hash.h"

namespace Dolas
{
	// Static member definition - stores hash-to-string mappings for reverse lookup
	std::unordered_map<UInt, std::string> HashConverter::s_hashToString;

	UInt HashConverter::StringHash(const std::string& str)
	{
		// FNV-1a (Fowler-Noll-Vo) hash algorithm implementation
		// This is a non-cryptographic hash function that's fast and has good distribution
		// properties, making it ideal for hash tables and resource identification in games
		//
		// Algorithm steps:
		// 1. Start with the FNV offset basis
		// 2. For each byte in the input:
		//    a. XOR the byte with the current hash value
		//    b. Multiply by the FNV prime
		// 3. Return the final hash value
		
		const UInt FNV_OFFSET_BASIS = 2166136261U;  // Standard FNV-1a offset basis for 32-bit
		const UInt FNV_PRIME = 16777619U;           // Standard FNV-1a prime for 32-bit
		
		UInt hash = FNV_OFFSET_BASIS;
		
		// Process each character in the string
		for (char c : str)
		{
			hash ^= static_cast<UInt>(c);  // XOR with current byte
			hash *= FNV_PRIME;             // Multiply by FNV prime for good distribution
		}

		// In debug builds, automatically register the string for reverse lookup
		// This allows debugging and logging systems to convert hashes back to readable strings
		// #if defined(DEBUG) || defined(_DEBUG)
		s_hashToString[hash] = str;
		// #endif
		
		return hash;
	}

	//void HashConverter::RegisterString(const std::string& str)
	//{
	//	// Manually register a string for reverse lookup
	//	// This function duplicates the hash calculation to avoid recursive calls
	//	// with the StringHash function during debug mode auto-registration
	//	
	//	const UInt FNV_OFFSET_BASIS = 2166136261U;
	//	const UInt FNV_PRIME = 16777619U;
	//	
	//	UInt hash = FNV_OFFSET_BASIS;
	//	for (char c : str)
	//	{
	//		hash ^= static_cast<UInt>(c);
	//		hash *= FNV_PRIME;
	//	}
	//	
	//	// Store the mapping for later reverse lookup
	//	s_hashToString[hash] = str;
	//}

	std::string HashConverter::GetString(UInt hash)
	{
		// Attempt to find the original string for the given hash
		auto it = s_hashToString.find(hash);
		if (it != s_hashToString.end())
		{
			// Hash found - return the original string
			return it->second;
		}
		
		// Hash not found - return a debug-friendly format showing the hash value
		// This helps identify which resources are missing from the registry
		return "Unknown[" + std::to_string(hash) + "]";
	}

	Bool HashConverter::HasString(UInt hash)
	{
		// Check if a hash exists in the registry without creating a string
		// This is more efficient than GetString() when you only need to check existence
		return s_hashToString.find(hash) != s_hashToString.end();
	}

	void HashConverter::ClearRegistry()
	{
		// Remove all hash-to-string mappings from memory
		// Useful for:
		// - Memory management when switching between different resource sets
		// - Unit testing to ensure clean state between tests
		// - Runtime cleanup when reverse lookup is no longer needed
		s_hashToString.clear();
	}
}