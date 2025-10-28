#include "core/dolas_engine.h"

int main()
{
	if (Dolas::g_dolas_engine.Initialize())
    {
        Dolas::g_dolas_engine.Run();
    }
    Dolas::g_dolas_engine.Clear();
    return 0;
}