#include "dolas_timer_manager.h"
#include <chrono>

namespace Dolas
{
    TimerManager::TimerManager()
    {
    }
    TimerManager::~TimerManager()
    {
    }
    bool TimerManager::Initialize()
    {
        m_last_time_us = GetMicroSecondsTime();
        return true;
    }
    void TimerManager::Clear()
    {
        m_last_time_us = 0;
        m_delta_time_ms = 0;
    }
    void TimerManager::Tick()
    {
        long long current_time_us = GetMicroSecondsTime();
        m_delta_time_ms = static_cast<Float>(current_time_us - m_last_time_us) / 1000.0f;
        m_last_time_us = current_time_us;
        m_frame_count++;
        m_frame_per_second = 1000.0f / m_delta_time_ms;
    }
    long long TimerManager::GetMicroSecondsTime() const
    {
        // Get current time point using high resolution clock
        auto now = std::chrono::high_resolution_clock::now();
        // Convert to milliseconds since epoch
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()
        );
        return us.count();
    }
    Float TimerManager::GetDeltaTime() const
    {
        return m_delta_time_ms;
    }
    Float TimerManager::GetFPS() const
    {
        return m_frame_per_second;
    }
} // namespace dolas