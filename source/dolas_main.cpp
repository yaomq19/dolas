#include "dolas.h"

int main(int argc, char** argv)
{
    Dolas::Dolas dolas;

    if (dolas.initialize())
    {
        dolas.run();
    }

    dolas.destroy();
    return 0;
}