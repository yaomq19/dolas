#include "dolas.h"

int main(int argc, char** argv)
{
    Dolas::Dolas dolas;
    dolas.initialize();

    if (dolas.isInitialized())
    {
        dolas.run();
    }

    dolas.destroy();
    return 0;
}