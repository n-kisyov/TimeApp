#include "MainWindow.h"
#include "../TimeApplication.h"
#include "DarkTheme.h"
#include <sstream>
#include <iomanip>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

MainWindow::MainWindow(TimeApplication& app)
    : app_(app)
    , is_initialized_(false)
    , countdown_input_minutes_(5)
    , countdown_input_seconds_(0)
    , show_milliseconds_(false)
    , hwnd_(nullptr)
    , pd3dDevice_(nullptr)
    , pd3dDeviceContext_(nullptr)
    , pSwapChain_(nullptr)
    , pMainRenderTargetView_(nullptr)
    , done_(false) {
}

MainWindow::~MainWindow() {
    Shutdown();
}

bool MainWindow::Initialize() {
    // Create window
    if (!CreateAppWindow()) {  // Updated method name
        return false;
    }
    
    // Create D3D11 device
    if (!CreateDeviceD3D()) {
        CleanupWindow();
        return false;
    }
    
    // Show the window
    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    DarkTheme::Apply();
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX11_Init(pd3dDevice_, pd3dDeviceContext_);
    
    is_initialized_ = true;
    return true;
}

bool MainWindow::CreateAppWindow() {  // Renamed method
    // Create application window
    wc_ = { sizeof(wc_), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TimeApp", nullptr };
    RegisterClassExW(&wc_);
    
    // Use CreateWindowW explicitly to avoid macro issues
    hwnd_ = CreateWindowW(wc_.lpszClassName, L"Time Display Application", 
                         WS_OVERLAPPEDWINDOW, 100, 100, 500, 400, 
                         nullptr, nullptr, wc_.hInstance, nullptr);
    
    if (!hwnd_) {
        return false;
    }
    
    // Store this pointer for use in WindowProc
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    return true;
}

// Rest of the methods remain the same as previously provided...
bool MainWindow::CreateDeviceD3D() {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.BufferCount = 2;
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.Stereo = FALSE;
    
    // Create device and swap chain
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    
    IDXGIFactory2* dxgiFactory = nullptr;
    IDXGIDevice* dxgiDevice = nullptr;
    IDXGIAdapter* dxgiAdapter = nullptr;
    
    HRESULT res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, 
                                   featureLevelArray, 2, D3D11_SDK_VERSION, 
                                   &pd3dDevice_, &featureLevel, &pd3dDeviceContext_);
    
    if (res != S_OK) return false;
    
    pd3dDevice_->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
    
    dxgiFactory->CreateSwapChainForHwnd(pd3dDevice_, hwnd_, &sd, nullptr, nullptr, &pSwapChain_);
    
    dxgiFactory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();
    
    // Create render target
    ID3D11Texture2D* pBackBuffer;
    pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    pd3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &pMainRenderTargetView_);
    pBackBuffer->Release();
    
    return true;
}

void MainWindow::Render() {
    if (!is_initialized_) return;
    
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    // Main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
                                   ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoMove;
    
    if (ImGui::Begin("Time Display Application", nullptr, window_flags)) {
        
        RenderTimeDisplay();
        ImGui::Separator();
        
        RenderNTPControls();
        ImGui::Separator();
        
        if (ImGui::BeginTabBar("TimerTabs")) {
            if (ImGui::BeginTabItem("Stopwatch")) {
                RenderStopwatch();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Countdown")) {
                RenderCountdown();
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        RenderStatusBar();
    }
    ImGui::End();
    
    // Rendering
    ImGui::Render();
    const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    pd3dDeviceContext_->OMSetRenderTargets(1, &pMainRenderTargetView_, nullptr);
    pd3dDeviceContext_->ClearRenderTargetView(pMainRenderTargetView_, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    pSwapChain_->Present(1, 0); // Present with vsync
}

bool MainWindow::ProcessEvents() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            done_ = true;
        }
    }
    return !done_;
}

// Window procedure
LRESULT CALLBACK MainWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    
    MainWindow* window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    
    switch (msg) {
        case WM_SIZE:
            if (window && window->pd3dDevice_ != nullptr && wParam != SIZE_MINIMIZED) {
                if (window->pMainRenderTargetView_) {
                    window->pMainRenderTargetView_->Release();
                    window->pMainRenderTargetView_ = nullptr;
                }
                window->pSwapChain_->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                
                ID3D11Texture2D* pBackBuffer;
                window->pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
                window->pd3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &window->pMainRenderTargetView_);
                pBackBuffer->Release();
            }
            return 0;
            
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void MainWindow::CleanupDeviceD3D() {
    if (pMainRenderTargetView_) { pMainRenderTargetView_->Release(); pMainRenderTargetView_ = nullptr; }
    if (pSwapChain_) { pSwapChain_->Release(); pSwapChain_ = nullptr; }
    if (pd3dDeviceContext_) { pd3dDeviceContext_->Release(); pd3dDeviceContext_ = nullptr; }
    if (pd3dDevice_) { pd3dDevice_->Release(); pd3dDevice_ = nullptr; }
}

void MainWindow::CleanupWindow() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
}

void MainWindow::Shutdown() {
    if (is_initialized_) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        
        CleanupDeviceD3D();
        CleanupWindow();
        
        is_initialized_ = false;
    }
}

// All the render methods remain the same as in my previous response
void MainWindow::RenderTimeDisplay() {
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", FormatCurrentTime().c_str());
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::Checkbox("Show Milliseconds", &show_milliseconds_);
}

void MainWindow::RenderNTPControls() {
    if (ImGui::Button("Sync with NTP", ImVec2(120, 0))) {
        app_.SyncTimeWithNTP();
    }
    
    ImGui::SameLine();
    if (app_.IsNTPSyncInProgress()) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Syncing...");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Ready");
    }
}

void MainWindow::RenderStopwatch() {
    auto& stopwatch = app_.GetStopwatch();
    
    ImGui::SetWindowFontScale(1.5f);
    if (show_milliseconds_) {
        ImGui::Text("%s", stopwatch.FormatTimeWithMilliseconds().c_str());
    } else {
        ImGui::Text("%s", stopwatch.FormatTime().c_str());
    }
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::Spacing();
    HandleStopwatchControls();
}

void MainWindow::RenderCountdown() {
    auto& countdown = app_.GetCountdown();
    
    ImGui::Text("Set Countdown:");
    ImGui::PushItemWidth(80);
    ImGui::InputInt("Minutes", &countdown_input_minutes_, 1, 10);
    ImGui::SameLine();
    ImGui::InputInt("Seconds", &countdown_input_seconds_, 1, 10);
    ImGui::PopItemWidth();
    
    countdown_input_minutes_ = std::max(0, countdown_input_minutes_);
    countdown_input_seconds_ = std::max(0, std::min(59, countdown_input_seconds_));
    
    ImGui::Spacing();
    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("%s", countdown.FormatTime().c_str());
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::Spacing();
    HandleCountdownControls();
}

void MainWindow::HandleStopwatchControls() {
    auto& stopwatch = app_.GetStopwatch();
    
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
}

void MainWindow::HandleCountdownControls() {
    auto& countdown = app_.GetCountdown();
    
    switch (countdown.GetState()) {
        case Timer::State::Stopped:
            if (ImGui::Button("Start Countdown", ImVec2(120, 0))) {
                auto total_seconds = countdown_input_minutes_ * 60 + countdown_input_seconds_;
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
}

void MainWindow::RenderStatusBar() {
    ImGui::Separator();
    ImGui::Text("Status: Application Running");
}

std::string MainWindow::FormatCurrentTime() const {
    auto time_point = app_.GetCurrentTime();
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    if (show_milliseconds_) {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            time_point.time_since_epoch()) % 1000;
        oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    }
    
    return oss.str();
}
