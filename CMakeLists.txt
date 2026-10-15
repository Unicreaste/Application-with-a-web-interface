cmake_minimum_required(VERSION 3.16)

project(ApplicationWithWebInterface LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    WebEngineWidgets
    WebChannel
)

find_package(
    SQLite3 REQUIRED
)

add_executable(myapp
    main.cpp

    src/mainwindow.h
    src/mainwindow.cpp

    src/resizehandle.h
    src/resizehandle.cpp

    src/glowbutton.h
    src/glowbutton.cpp

    src/dragbar.h
    src/dragbar.cpp

    src/titlebar.h
    src/titlebar.cpp

    src/filewatcher.h
    src/filewatcher.cpp

    src/pageloader.h
    src/pageloader.cpp

    src/database_manager.h
    src/database_manager.cpp

    src/task_positions.h
    src/task_positions.cpp

    src/chat_manager.h
    src/chat_manager.cpp

    src/theme_manager.cpp
    src/theme_manager.h
)

target_link_libraries(myapp PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
    SQLite::SQLite3
    Qt6::WebChannel
)

target_include_directories(myapp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(myapp PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/pch.h
    )
endif()