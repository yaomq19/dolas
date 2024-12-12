#include "dolas_timer.h"
namespace Dolas
{
Timer::Timer()
{

}

Timer::~Timer()
{

}

void Timer::tick()
{
    // Do nothing
}

Float Timer::getDeltaTime() const
{
    return m_delta_time;
}
}