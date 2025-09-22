#include "base/dolas_paths.h"
#include "core/dolas_engine.h"

int main()
{
	if (Dolas::g_dolas_engine.Initialize())
    {
        // Dolas::g_dolas_engine.Run();
        Dolas::g_dolas_engine.Run();
    }
    
    // 程序退出前清理所有资源
    Dolas::g_dolas_engine.Clear();
    return 0;
}