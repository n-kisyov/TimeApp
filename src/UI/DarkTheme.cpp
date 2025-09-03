#include "DarkTheme.h"

#ifdef USE_IMGUI
#include "imgui.h"
#endif

void DarkTheme::Apply() {
#ifdef USE_IMGUI
    ApplyImGuiTheme();
#else
    ApplyWin32Theme();
#endif
}

#ifdef USE_IMGUI
void DarkTheme::ApplyImGuiTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Rounding
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    
    // Spacing
    style.WindowPadding = ImVec2(10, 10);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 6);
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]);
    colors[ImGuiCol_Text] = ImVec4(TEXT_COLOR[0], TEXT_COLOR[1], TEXT_COLOR[2], TEXT_COLOR[3]);
    colors[ImGuiCol_Button] = ImVec4(BUTTON_COLOR[0], BUTTON_COLOR[1], BUTTON_COLOR[2], BUTTON_COLOR[3]);
    colors[ImGuiCol_ButtonHovered] = ImVec4(BUTTON_HOVER_COLOR[0], BUTTON_HOVER_COLOR[1], BUTTON_HOVER_COLOR[2], BUTTON_HOVER_COLOR[3]);
    colors[ImGuiCol_ButtonActive] = ImVec4(BUTTON_ACTIVE_COLOR[0], BUTTON_ACTIVE_COLOR[1], BUTTON_ACTIVE_COLOR[2], BUTTON_ACTIVE_COLOR[3]);
    colors[ImGuiCol_FrameBg] = ImVec4(FRAME_COLOR[0], FRAME_COLOR[1], FRAME_COLOR[2], FRAME_COLOR[3]);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(FRAME_COLOR[0] + 0.1f, FRAME_COLOR[1] + 0.1f, FRAME_COLOR[2] + 0.1f, FRAME_COLOR[3]);
    colors[ImGuiCol_FrameBgActive] = ImVec4(FRAME_COLOR[0] - 0.1f, FRAME_COLOR[1] - 0.1f, FRAME_COLOR[2] - 0.1f, FRAME_COLOR[3]);
    colors[ImGuiCol_TitleBg] = ImVec4(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(BACKGROUND_COLOR[0] + 0.05f, BACKGROUND_COLOR[1] + 0.05f, BACKGROUND_COLOR[2] + 0.05f, 1.0f);
}
#endif

void DarkTheme::ApplyWin32Theme() {
    // Implementation for Win32 dark mode
    // This would involve setting window class styles and custom drawing
}
