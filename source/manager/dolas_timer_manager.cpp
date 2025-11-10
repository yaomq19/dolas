#include "dolas_timer_manager.h"
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace Dolas
{
    TimerManager::TimerManager()
        : m_last_time_us(0)
        , m_delta_time_ms(0)
        , m_frame_count(0)
        , m_frame_per_second(0)
        , m_max_fps(0.0f) // 0 = unlimited
    {
    }
    TimerManager::~TimerManager()
    {
    }
    bool TimerManager::Initialize()
    {
        m_last_time_us = GetMicroSecondsTime();
        
#ifdef _WIN32
        // Set Windows timer resolution to 1ms for better sleep precision
        // This improves frame rate stability significantly
        timeBeginPeriod(1);
#endif
        
        return true;
    }
    void TimerManager::Clear()
    {
#ifdef _WIN32
        // Restore default Windows timer resolution
        timeEndPeriod(1);
#endif
        
        m_last_time_us = 0;
        m_delta_time_ms = 0;
        m_frame_count = 0;
        m_frame_per_second = 0;
    }

    void TimerManager::Tick()
    {
        // First, limit frame rate if needed (before calculating delta time)
        if (m_max_fps > 0.0f)
        {
            const Float target_frame_time_ms = 1000.0f / m_max_fps;
            
            // Keep checking until we reach the target frame time
            while (true)
            {
                long long current_time_us = GetMicroSecondsTime();
                Float elapsed_ms = static_cast<Float>(current_time_us - m_last_time_us) / 1000.0f;
                
                if (elapsed_ms >= target_frame_time_ms)
                {
                    break; // Target frame time reached
                }
                
                Float remaining_time_ms = target_frame_time_ms - elapsed_ms;
                
                // Use sleep only if we have significant time remaining (> 2ms)
                // With timeBeginPeriod(1), sleep(1) should be accurate to ~1ms
                if (remaining_time_ms > 2.0f)
                {
                    // Sleep for a conservative amount, leaving time for busy wait
                    int sleep_ms = static_cast<int>(remaining_time_ms - 2.0f);
                    if (sleep_ms > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
                    }
                }
                // Busy wait for the remaining time (last ~2ms) for precision
            }
        }
        
        // Now calculate the actual delta time
        long long current_time_us = GetMicroSecondsTime();
        m_delta_time_ms = static_cast<Float>(current_time_us - m_last_time_us) / 1000.0f;
        m_last_time_us = current_time_us;
        m_frame_per_second = 1000.0f / m_delta_time_ms;
        m_frame_count++;
        
        // Debug: Print FPS every 100 frames to verify stability
        if (m_frame_count % 100 == 0)
        {
            printf("Frame %lld: FPS = %.2f, Delta = %.3f ms (Target: %.2f FPS)\n", 
                   m_frame_count, m_frame_per_second, m_delta_time_ms, m_max_fps);
        }
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

    long long TimerManager::GetFrameCount() const
    {
        return m_frame_count;
    }

    void TimerManager::SetMaxFPS(Float max_fps)
    {
        m_max_fps = max_fps;
    }

    Float TimerManager::GetMaxFPS() const
    {
        return m_max_fps;
    }

    void TimerManager::LimitFrameRate()
    {
        // This function is now empty - frame rate limiting is done in Tick()
        // Kept for backward compatibility
    }

} // namespace dolas