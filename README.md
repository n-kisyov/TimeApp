Simple Windows Clock app with NTP sync, Timer, Stopwatch and more



This is using the imgui library - https://github.com/ocornut/imgui

Download from Release page or compile from source

Build

  - Download Microsoft C++ Build Tools from the official Microsoft website
  - Run the installer
  - Select "C++ build tools" workload
  - Ensure these components are selected:

    MSVC v143 - VS 2022 C++ x64/x86 build tools
    Windows 11 SDK (latest version)
    CMake tools for Visual Studio

  - Click Install
  - Install CMake, Download from: https://cmake.org/download/
  - Choose "Windows x64 Installer"
  - During installation, select "Add CMake to system PATH"


mkdir build
cd build

cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release


