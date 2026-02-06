#ifndef DOLAS_TRANSFORM_H
#define DOLAS_TRANSFORM_H
#
#include "dolas_math.h"

namespace Dolas
{
    struct Transform
    {
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
    };
}

#endif // DOLAS_TRANSFORM_H