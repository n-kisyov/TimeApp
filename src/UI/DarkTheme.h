#pragma once

// Forward declarations for different UI frameworks
#ifdef USE_IMGUI
struct ImGuiStyle;
#endif

class DarkTheme {
public:
    static void Apply();
    
    // Color constants
    static constexpr float BACKGROUND_COLOR[4] = {0.1f, 0.1f, 0.1f, 0.94f};
    static constexpr float TEXT_COLOR[4] = {0.9f, 0.9f, 0.9f, 1.0f};
    static constexpr float BUTTON_COLOR[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    static constexpr float BUTTON_HOVER_COLOR[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    static constexpr float BUTTON_ACTIVE_COLOR[4] = {0.15f, 0.15f, 0.15f, 1.0f};
    static constexpr float FRAME_COLOR[4] = {0.25f, 0.25f, 0.25f, 1.0f};
    
private:
    static void ApplyImGuiTheme();
    static void ApplyWin32Theme();
};
