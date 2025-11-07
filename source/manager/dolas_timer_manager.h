#ifndef __DOLAS_TIMER_MANAGER_H__
#define __DOLAS_TIMER_MANAGER_H__

namespace Dolas
{
    class TimerManager
    {
    public:
        TimerManager();
        ~TimerManager();
        void Tick();
        long long GetTime() const; // unit : milliseconds
        long long GetDeltaTime() const; // unit : milliseconds
        long long GetFPS() const;
        long long GetFrameCount() const;
    };
} // namespace dolas

#endif // __DOLAS_TIMER_MANAGER_H__