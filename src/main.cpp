#include "WindowsHeaders.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "TimeApplication.h"
#include <iostream>
#include <sstream>     // For std::ostringstream
#include <iomanip>     // For std::put_time, std::setfill, std::setw
#include <chrono>

// Global variables
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Debug console
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    
    std::cout << "Starting TimeApp with full functionality..." << std::endl;

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TimeApp", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Time Display Application", WS_OVERLAPPEDWINDOW, 100, 100, 600, 500, nullptr, nullptr, wc.hInstance, nullptr);

    if (!hwnd) {
        std::cout << "Failed to create window!" << std::endl;
        return -1;
    }

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        std::cout << "Failed to create D3D device!" << std::endl;
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui dark style
    ImGui::StyleColorsDark();
    
    // Customize colors for better dark theme
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.94f);
    colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    std::cout << "ImGui initialized successfully!" << std::endl;

    // Initialize TimeApplication
    TimeApplication app;
    std::cout << "TimeApplication initialized!" << std::endl;

    // UI state variables
    int countdown_minutes = 5;
    int countdown_seconds = 0;
    bool show_milliseconds = true;

    // Main loop
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Get current time from TimeApplication
        auto now = app.GetCurrentTime();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        if (show_milliseconds) {
            oss << "." << std::setfill('0') << std::setw(3) << ms.count();
        }

        // Create main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Time Display Application", nullptr, 
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
        
        // Large time display
        ImGui::SetWindowFontScale(2.5f);
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", oss.str().c_str());
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::Checkbox("Show Milliseconds", &show_milliseconds);
        ImGui::Separator();
        
        // NTP Controls
        if (ImGui::Button("Sync with NTP", ImVec2(120, 0))) {
            app.SyncTimeWithNTP();
            std::cout << "NTP sync requested" << std::endl;
        }
        
        ImGui::SameLine();
        if (app.IsNTPSyncInProgress()) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Syncing...");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Ready");
        }
        
        ImGui::Separator();
        
        // Tabbed interface for timers
        if (ImGui::BeginTabBar("TimerTabs")) {
            if (ImGui::BeginTabItem("Stopwatch")) {
                auto& stopwatch = app.GetStopwatch();
                
                // Display stopwatch time
                ImGui::SetWindowFontScale(1.8f);
                if (show_milliseconds) {
                    ImGui::Text("%s", stopwatch.FormatTimeWithMilliseconds().c_str());
                } else {
                    ImGui::Text("%s", stopwatch.FormatTime().c_str());
                }
                ImGui::SetWindowFontScale(1.0f);
                
                ImGui::Spacing();
                
                // Stopwatch controls
                switch (stopwatch.GetState()) {
                    case Timer::State::Stopped:
                        if (ImGui::Button("Start", ImVec2(80, 0))) {
                            stopwatch.Start();
                        }
                        break;
                        
                    case Timer::State::Running:
                        if (ImGui::Button("Pause", ImVec2(80, 0))) {
                            stopwatch.Pause();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Reset", ImVec2(80, 0))) {
                            stopwatch.Reset();
                        }
                        break;
                        
                    case Timer::State::Paused:
                        if (ImGui::Button("Resume", ImVec2(80, 0))) {
                            stopwatch.Resume();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Reset", ImVec2(80, 0))) {
                            stopwatch.Reset();
                        }
                        break;
                }
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Countdown")) {
                auto& countdown = app.GetCountdown();
                
                // Input for countdown duration
                ImGui::Text("Set Countdown:");
                ImGui::PushItemWidth(80);
                ImGui::InputInt("Minutes", &countdown_minutes, 1, 10);
                ImGui::SameLine();
                ImGui::InputInt("Seconds", &countdown_seconds, 1, 10);
                ImGui::PopItemWidth();
                
                // Clamp values
                countdown_minutes = std::max(0, countdown_minutes);
                countdown_seconds = std::max(0, std::min(59, countdown_seconds));
                
                // Display countdown time
                ImGui::Spacing();
                ImGui::SetWindowFontScale(1.8f);
                ImGui::Text("%s", countdown.FormatTime().c_str());
                ImGui::SetWindowFontScale(1.0f);
                
                ImGui::Spacing();
                
                // Countdown controls
                switch (countdown.GetState()) {
                    case Timer::State::Stopped:
                        if (ImGui::Button("Start Countdown", ImVec2(120, 0))) {
                            auto total_seconds = countdown_minutes * 60 + countdown_seconds;
                            countdown.SetDuration(std::chrono::seconds(total_seconds));
                            countdown.Start();
                        }
                        break;
                        
                    case Timer::State::Running:
                        if (ImGui::Button("Pause", ImVec2(80, 0))) {
                            countdown.Pause();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Stop", ImVec2(80, 0))) {
                            countdown.Stop();
                        }
                        break;
                        
                    case Timer::State::Paused:
                        if (ImGui::Button("Resume", ImVec2(80, 0))) {
                            countdown.Resume();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Stop", ImVec2(80, 0))) {
                            countdown.Stop();
                        }
                        break;
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        // Status bar
        ImGui::Separator();
        ImGui::Text("Status: Application Running | FPS: %.1f", io.Framerate);
        
        ImGui::End();

        // Rendering
        ImGui::Render();
        const float clear_color[11] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    std::cout << "Application closed successfully!" << std::endl;
    return 0;
}

// D3D Helper functions (same as working version)
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            if (g_pd3dDevice != nullptr) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
