#pragma once

#include "WindowsHeaders.h"
#include "NTPClient.h"
#include "Timer.h"
#include <memory>
#include <chrono>

class TimeApplication {
public:
    TimeApplication();
    ~TimeApplication();
    
    // Time management
    std::chrono::system_clock::time_point GetCurrentTime() const;
    void SyncTimeWithNTP();
    bool IsNTPSyncInProgress() const;
    
    // Timer access
    Timer& GetStopwatch() { return stopwatch_; }
    Timer& GetCountdown() { return countdown_; }
    
private:
    std::unique_ptr<NTPClient> ntp_client_;
    
    Timer stopwatch_;
    Timer countdown_;
    
    std::chrono::system_clock::time_point current_time_;
    std::chrono::system_clock::time_point last_ntp_sync_;
    
    bool is_ntp_sync_in_progress_;
    
    static const std::chrono::minutes NTP_SYNC_INTERVAL;
};
