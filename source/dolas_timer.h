#ifndef DOLAS_TIMER_H
#define DOLAS_TIMER_H
#include "dolas_common.h"
namespace Dolas
{
class Timer
{
public:
    Timer();
    ~Timer();

    void tick();
    Float getDeltaTime() const;
private:
    Float m_delta_time;
};
}
#endif