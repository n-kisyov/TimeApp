#pragma once

#include "../WindowsHeaders.h"  // Use common header
#include <string>
#include <chrono>

class TimeApplication; // Forward declaration

class MainWindow {
public:
    explicit MainWindow(TimeApplication& app);
    ~MainWindow();
    
    bool Initialize();
    void Render();
    bool ProcessEvents();
    void Shutdown();
    
    HWND GetWindowHandle() const { return hwnd_; }
    
private:
    void RenderTimeDisplay();
    void RenderNTPControls();
    void RenderStopwatch();
    void RenderCountdown();
    void RenderStatusBar();
    
    std::string FormatCurrentTime() const;
    void HandleStopwatchControls();
    void HandleCountdownControls();
    
    bool CreateDeviceD3D();
    void CleanupDeviceD3D();
    bool CreateAppWindow();
    void CleanupWindow();
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    TimeApplication& app_;
    bool is_initialized_;
    
    // Window handles
    HWND hwnd_;
    WNDCLASSEXW wc_;
    
    // DirectX 11
    ID3D11Device* pd3dDevice_;
    ID3D11DeviceContext* pd3dDeviceContext_;
    IDXGISwapChain1* pSwapChain_;
    ID3D11RenderTargetView* pMainRenderTargetView_;
    
    // UI state
    int countdown_input_minutes_;
    int countdown_input_seconds_;
    bool show_milliseconds_;
    bool done_;
};
