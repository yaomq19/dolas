#ifndef DOLAS_COLOR_H
#define DOLAS_COLOR_H
#include "dolas_math.h"
namespace Dolas
{
    struct Color
    {
        Float r;
        Float g;
        Float b;
        Float a;

        const static Color WHITE;
        const static Color BLACK;
        const static Color RED;
        const static Color GREEN;
        const static Color BLUE;
        const static Color YELLOW;
        const static Color CYAN;
        const static Color MAGENTA;
    };

    
}// namespace Dolas
#endif // DOLAS_COLOR_H