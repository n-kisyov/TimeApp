#include "TimeApplication.h"
#include <iostream>
#include <thread>

const std::chrono::minutes TimeApplication::NTP_SYNC_INTERVAL{30};

TimeApplication::TimeApplication() 
    : is_ntp_sync_in_progress_(false)
    , stopwatch_(Timer::Type::Stopwatch)
    , countdown_(Timer::Type::Countdown) {
    
    // Initialize NTP client
    ntp_client_ = std::make_unique<NTPClient>();
    
    // Set initial time
    current_time_ = std::chrono::system_clock::now();
    last_ntp_sync_ = std::chrono::system_clock::now();
    
    // Perform initial NTP sync
    SyncTimeWithNTP();
    
    std::cout << "TimeApplication initialized successfully" << std::endl;
}

TimeApplication::~TimeApplication() {
    // Cleanup handled by smart pointers automatically
}

std::chrono::system_clock::time_point TimeApplication::GetCurrentTime() const {
    // Update current time each call to ensure accuracy
    return std::chrono::system_clock::now();
}

void TimeApplication::SyncTimeWithNTP() {
    if (is_ntp_sync_in_progress_ || !ntp_client_) return;
    
    is_ntp_sync_in_progress_ = true;
    
    // Async NTP sync to avoid blocking UI
    std::thread([this]() {
        try {
            if (ntp_client_->SyncTime()) {
                last_ntp_sync_ = std::chrono::system_clock::now();
                std::cout << "NTP sync successful" << std::endl;
            } else {
                std::cout << "NTP sync failed" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "NTP sync error: " << e.what() << std::endl;
        }
        is_ntp_sync_in_progress_ = false;
    }).detach();
}

bool TimeApplication::IsNTPSyncInProgress() const {
    return is_ntp_sync_in_progress_;
}
