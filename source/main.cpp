#include "base/dolas_paths.h"
#include "core/dolas_engine.h"

int main()
{
    if (Dolas::g_dolas_engine.Initialize())
    {
        Dolas::g_dolas_engine.Run();
    }
    return 0;
}