#include "dolas_engine.h"
#include "base/dolas_paths.h"

int main()
{
    // 打印项目路径信息
    Dolas::Paths::PrintPathInfo();
    
    if (Dolas::g_dolas_engine.Initialize())
    {
        Dolas::g_dolas_engine.Run();
    }
    return 0;
}