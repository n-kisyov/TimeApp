#pragma once

#include "WindowsHeaders.h"  // Use common header
#include <string>
#include <vector>
#include <chrono>
#include <memory>

class NTPClient {
    // Rest of your class remains the same...
public:
    struct NTPResult {
        bool success = false;
        std::chrono::system_clock::time_point synced_time;
        std::chrono::milliseconds round_trip_delay{0};
        std::string error_message;
    };
    
    NTPClient();
    ~NTPClient();
    
    bool SyncTime();
    NTPResult QueryServer(const std::string& server, int timeout_ms = 5000);
    
    void AddServer(const std::string& server);
    void SetDefaultServers();
    
    std::chrono::system_clock::time_point GetLastSyncTime() const;
    bool IsConnected() const;
    
private:
    struct NTPPacket {
        uint32_t li_vn_mode;
        uint32_t stratum;
        uint32_t poll;
        uint32_t precision;
        uint32_t root_delay;
        uint32_t root_dispersion;
        uint32_t ref_id;
        uint64_t ref_timestamp;
        uint64_t orig_timestamp;
        uint64_t recv_timestamp;
        uint64_t trans_timestamp;
    };
    
    bool InitializeWinsock();
    void CleanupWinsock();
    
    NTPResult SendNTPRequest(const std::string& server, int timeout_ms);
    uint64_t GetNTPTimestamp() const;
    std::chrono::system_clock::time_point NTPToSystemTime(uint64_t ntp_time) const;
    
    std::vector<std::string> ntp_servers_;
    std::chrono::system_clock::time_point last_sync_time_;
    bool winsock_initialized_;
    bool is_connected_;
};

