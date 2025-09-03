#pragma once

#include <chrono>
#include <functional>

class Timer {
public:
    enum class Type {
        Stopwatch,
        Countdown
    };
    
    enum class State {
        Stopped,
        Running,
        Paused
    };
    
    explicit Timer(Type type);
    
    void Start();
    void Stop();
    void Pause();
    void Resume();
    void Reset();
    
    void SetDuration(std::chrono::seconds duration); // For countdown
    void Update();
    
    std::chrono::milliseconds GetElapsedTime() const;
    std::chrono::seconds GetRemainingTime() const;
    
    State GetState() const { return state_; }
    Type GetType() const { return type_; }
    
    void SetOnFinished(std::function<void()> callback);
    
    // Formatting helpers
    std::string FormatTime() const;
    std::string FormatTimeWithMilliseconds() const;
    
private:
    Type type_;
    State state_;
    
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point pause_time_;
    std::chrono::milliseconds accumulated_time_{0};
    
    std::chrono::seconds countdown_duration_{0};
    std::function<void()> on_finished_callback_;
    
    void CheckCountdownFinished();
};
