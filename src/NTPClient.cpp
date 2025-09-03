#include "NTPClient.h"
#include <iostream>
#include <thread>

// Remove all Windows/Winsock includes since they're in WindowsHeaders.h

NTPClient::NTPClient() 
    : winsock_initialized_(false)
    , is_connected_(false) {
    
    SetDefaultServers();
    InitializeWinsock();
}

NTPClient::~NTPClient() {
    CleanupWinsock();
}

void NTPClient::SetDefaultServers() {
    ntp_servers_ = {
        "pool.ntp.org",
        "time.windows.com",
        "time.google.com",
        "time.cloudflare.com"
    };
}

bool NTPClient::InitializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    winsock_initialized_ = (result == 0);
    return winsock_initialized_;
}

void NTPClient::CleanupWinsock() {
    if (winsock_initialized_) {
        WSACleanup();
        winsock_initialized_ = false;
    }
}

bool NTPClient::SyncTime() {
    for (const auto& server : ntp_servers_) {
        auto result = QueryServer(server);
        if (result.success) {
            last_sync_time_ = result.synced_time;
            is_connected_ = true;
            return true;
        }
    }
    
    is_connected_ = false;
    return false;
}

NTPClient::NTPResult NTPClient::QueryServer(const std::string& server, int timeout_ms) {
    NTPResult result;
    
    if (!winsock_initialized_) {
        result.error_message = "Winsock not initialized";
        return result;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        result.error_message = "Failed to create socket";
        return result;
    }
    
    // Set timeout
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_ms, sizeof(timeout_ms));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout_ms, sizeof(timeout_ms));
    
    // Resolve server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(123); // NTP port
    
    hostent* host = gethostbyname(server.c_str());
    if (!host) {
        result.error_message = "Failed to resolve server address";
        closesocket(sock);
        return result;
    }
    
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
    
    // Create NTP packet
    NTPPacket packet{};
    packet.li_vn_mode = 0x1B; // LI=0, VN=3, Mode=3 (client)
    packet.trans_timestamp = GetNTPTimestamp();
    
    auto send_time = std::chrono::steady_clock::now();
    
    // Send request
    if (sendto(sock, (char*)&packet, sizeof(packet), 0, 
               (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        result.error_message = "Failed to send NTP request";
        closesocket(sock);
        return result;
    }
    
    // Receive response
    NTPPacket response{};
    int addr_len = sizeof(server_addr);
    if (recvfrom(sock, (char*)&response, sizeof(response), 0, 
                 (sockaddr*)&server_addr, &addr_len) == SOCKET_ERROR) {
        result.error_message = "Failed to receive NTP response";
        closesocket(sock);
        return result;
    }
    
    auto recv_time = std::chrono::steady_clock::now();
    result.round_trip_delay = std::chrono::duration_cast<std::chrono::milliseconds>(recv_time - send_time);
    
    closesocket(sock);
    
    // Convert NTP time to system time
    result.synced_time = NTPToSystemTime(response.trans_timestamp);
    result.success = true;
    
    return result;
}

uint64_t NTPClient::GetNTPTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    
    // NTP epoch starts at 1900, Unix epoch at 1970 (70 years difference)
    const uint64_t UNIX_TO_NTP_OFFSET = 2208988800ULL;
    
    return (seconds.count() + UNIX_TO_NTP_OFFSET) << 32;
}

std::chrono::system_clock::time_point NTPClient::NTPToSystemTime(uint64_t ntp_time) const {
    const uint64_t UNIX_TO_NTP_OFFSET = 2208988800ULL;
    
    uint64_t seconds = (ntp_time >> 32) - UNIX_TO_NTP_OFFSET;
    uint64_t fraction = ntp_time & 0xFFFFFFFF;
    
    auto time_point = std::chrono::system_clock::from_time_t(seconds);
    auto microseconds = (fraction * 1000000ULL) >> 32;
    
    return time_point + std::chrono::microseconds(microseconds);
}

std::chrono::system_clock::time_point NTPClient::GetLastSyncTime() const {
    return last_sync_time_;
}

bool NTPClient::IsConnected() const {
    return is_connected_;
}

void NTPClient::AddServer(const std::string& server) {
    ntp_servers_.push_back(server);
}
