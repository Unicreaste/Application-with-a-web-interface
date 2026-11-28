# Motion Desk


Requirements
C++ compiler
CMake
Qt 6 (Widgets, WebEngineWidgets)
SQLite3

---

Build and Run (Fedora Linux)
1. Install dependencies
    
        
    `sudo dnf install cmake gcc-c++ make sqlite-devel`


    `sudo dnf install qt6-qtbase-devel`


    `qt6-qtwebengine-devel`



2. Configure the project
`cmake -S . -B build`
3. Build
`cmake --build build -j$(nproc)`
4. Run
`./build/myapp`