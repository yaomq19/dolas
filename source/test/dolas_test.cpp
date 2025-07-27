#include "dolas_test.h"
#include <iostream>
#include "common/dolas_hash.h"

namespace Dolas
{
    void TestHash()
    {
        std::string str = "Hello, World!";
        UInt hash = HashConverter::StringHash(str);
        std::string str_cvt = HashConverter::GetString(hash); // This will print the string if in debug mode
        std::cout << "Hash: " << hash << std::endl;
		std::cout << "String from hash: " << str_cvt << std::endl;
    }
}