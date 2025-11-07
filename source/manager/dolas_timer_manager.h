#ifndef __DOLAS_TIMER_MANAGER_H__
#define __DOLAS_TIMER_MANAGER_H__

#include "base/dolas_base.h"

namespace Dolas
{
    class TimerManager
    {
    public:
        TimerManager();
        ~TimerManager();
        bool Initialize();
        void Clear();
        void Tick();
        long long GetMicroSecondsTime() const; // unit : microseconds
        Float GetDeltaTime() const; // unit : milliseconds
        Float GetFPS() const;
        long long GetFrameCount() const;
    protected:
        long long m_last_time_us; // unit : microseconds
        Float m_delta_time_ms; // unit : milliseconds
        long long m_frame_count;
        Float m_frame_per_second;
    };
} // namespace dolas

#endif // __DOLAS_TIMER_MANAGER_H__