#include "base/dolas_paths.h"
#include "core/dolas_engine.h"
#include "test/dolas_test.h"
void LaunchEngine()
{
    if (Dolas::g_dolas_engine.Initialize())
    {
        Dolas::g_dolas_engine.Run();
    }
}

int main()
{
    // Dolas::TestHash();
	LaunchEngine();
    return 0;
}