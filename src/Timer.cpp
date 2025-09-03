#include "Timer.h"
#include <sstream>
#include <iomanip>

Timer::Timer(Type type) 
    : type_(type)
    , state_(State::Stopped) {
}

void Timer::Start() {
    if (state_ == State::Running) return;
    
    start_time_ = std::chrono::steady_clock::now();
    state_ = State::Running;
    
    if (type_ == Type::Stopwatch) {
        accumulated_time_ = std::chrono::milliseconds{0};
    }
}

void Timer::Stop() {
    state_ = State::Stopped;
    accumulated_time_ = std::chrono::milliseconds{0};
}

void Timer::Pause() {
    if (state_ != State::Running) return;
    
    pause_time_ = std::chrono::steady_clock::now();
    accumulated_time_ += std::chrono::duration_cast<std::chrono::milliseconds>(
        pause_time_ - start_time_);
    state_ = State::Paused;
}

void Timer::Resume() {
    if (state_ != State::Paused) return;
    
    start_time_ = std::chrono::steady_clock::now();
    state_ = State::Running;
}

void Timer::Reset() {
    state_ = State::Stopped;
    accumulated_time_ = std::chrono::milliseconds{0};
}

void Timer::SetDuration(std::chrono::seconds duration) {
    countdown_duration_ = duration;
}

void Timer::Update() {
    if (state_ == State::Running && type_ == Type::Countdown) {
        CheckCountdownFinished();
    }
}

std::chrono::milliseconds Timer::GetElapsedTime() const {
    if (state_ == State::Stopped) {
        return std::chrono::milliseconds{0};
    }
    
    auto current_elapsed = accumulated_time_;
    
    if (state_ == State::Running) {
        auto now = std::chrono::steady_clock::now();
        current_elapsed += std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time_);
    }
    
    return current_elapsed;
}

std::chrono::seconds Timer::GetRemainingTime() const {
    if (type_ != Type::Countdown || state_ == State::Stopped) {
        return std::chrono::seconds{0};
    }
    
    auto elapsed = GetElapsedTime();
    auto remaining = countdown_duration_ - std::chrono::duration_cast<std::chrono::seconds>(elapsed);
    
    return std::max(std::chrono::seconds{0}, remaining);
}

void Timer::CheckCountdownFinished() {
    if (GetRemainingTime() == std::chrono::seconds{0}) {
        Stop();
        if (on_finished_callback_) {
            on_finished_callback_();
        }
    }
}

void Timer::SetOnFinished(std::function<void()> callback) {
    on_finished_callback_ = callback;
}

std::string Timer::FormatTime() const {
    std::chrono::seconds total_seconds;
    
    if (type_ == Type::Stopwatch) {
        total_seconds = std::chrono::duration_cast<std::chrono::seconds>(GetElapsedTime());
    } else {
        total_seconds = GetRemainingTime();
    }
    
    auto hours = std::chrono::duration_cast<std::chrono::hours>(total_seconds);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(total_seconds - hours);
    auto seconds = total_seconds - hours - minutes;
    
    std::ostringstream oss;
    if (hours.count() > 0) {
        oss << std::setfill('0') << std::setw(2) << hours.count() << ":";
    }
    oss << std::setfill('0') << std::setw(2) << minutes.count() << ":"
        << std::setfill('0') << std::setw(2) << seconds.count();
    
    return oss.str();
}

std::string Timer::FormatTimeWithMilliseconds() const {
    auto base_time = FormatTime();
    auto elapsed = GetElapsedTime();
    auto milliseconds = elapsed.count() % 1000;
    
    std::ostringstream oss;
    oss << base_time << "." << std::setfill('0') << std::setw(3) << milliseconds;
    
    return oss.str();
}
