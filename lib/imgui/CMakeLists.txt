cmake_minimum_required(VERSION 3.16)
project(TimeApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define USE_IMGUI to enable ImGui support
add_definitions(-DUSE_IMGUI)

# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Your application source files
set(APP_SOURCES
    src/main.cpp
    src/TimeApplication.cpp
    src/NTPClient.cpp
    src/Timer.cpp
    src/UI/DarkTheme.cpp
    src/UI/MainWindow.cpp
)

set(APP_HEADERS
    src/TimeApplication.h
    src/NTPClient.h
    src/Timer.h
    src/UI/DarkTheme.h
    src/UI/MainWindow.h
)

# Create executable
add_executable(TimeApp ${APP_SOURCES} ${APP_HEADERS} ${IMGUI_SOURCES})

# Include directories
target_include_directories(TimeApp PRIVATE 
    src/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link Windows libraries
target_link_libraries(TimeApp PRIVATE 
    ws2_32      # Winsock for networking
    winmm       # Windows multimedia
    d3d11       # DirectX 11
    dxgi        # DirectX Graphics Infrastructure
    d3dcompiler # DirectX shader compiler
)

# Compiler-specific options
if(MSVC)
    target_compile_options(TimeApp PRIVATE /W4)
    # Disable specific warnings for ImGui
    target_compile_definitions(TimeApp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
