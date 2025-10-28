#ifndef DOLAS_COLOR_H
#define DOLAS_COLOR_H
#include "core/dolas_math.h"
namespace Dolas
{
    struct Color
    {
        Float r;
        Float g;
        Float b;
        Float a;
    };

    const static Color COLOR_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
    const static Color COLOR_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
    const static Color COLOR_RED = { 1.0f, 0.0f, 0.0f, 1.0f };
    const static Color COLOR_GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
    const static Color COLOR_BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
    const static Color COLOR_YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
    const static Color COLOR_CYAN = { 0.0f, 1.0f, 1.0f, 1.0f };
    const static Color COLOR_MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f };
}// namespace Dolas
#endif // DOLAS_COLOR_H