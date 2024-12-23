#include "dolas.h"


int main(int argc, char** argv)
{
    

    if (Dolas::g_dolas.initialize())
    {
        Dolas::g_dolas.run();
    }

    Dolas::g_dolas.destroy();
    return 0;
}