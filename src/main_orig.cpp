#include "WindowsHeaders.h"  // Include this FIRST
#include "TimeApplication.h"
#include <iostream>

// Windows application entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // Hide console window for release builds
    #ifdef NDEBUG
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    #endif
    
    try {
        TimeApplication app;
        return app.Run();
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Application Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}

// Console entry point for debugging (optional)
#ifdef _CONSOLE
int main() {
    try {
        TimeApplication app;
        return app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return -1;
    }
}
#endif
