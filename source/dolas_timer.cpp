#include <chrono>
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
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::duration elapsed = now - m_last_time;
    m_delta_time = std::chrono::duration_cast<std::chrono::duration<Float>>(elapsed).count();
    m_last_time = now;
}

Float Timer::getDeltaTime() const
{
    return m_delta_time;
}
}