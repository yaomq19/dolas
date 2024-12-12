#include "dolas.h"

int main(int argc, char** argv)
{
    Dolas dolas;
    dolas.initialize();

    if (dolas.isInitialized())
    {
        dolas.run();
    }

    dolas.destroy();
    return 0;
}