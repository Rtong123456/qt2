cmake_minimum_required(VERSION 3.16)

project(TextEditor VERSION 1.0 LANGUAGES CXX)

# Qt5.12（实验环境）与 Qt6 均兼容
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 moc / uic / rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 优先查找 Qt6，找不到则查找 Qt5（实验要求 Qt 5.12.11）
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(TextEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    codeeditor.cpp
    codeeditor.h
    finddialog.cpp
    finddialog.h
    replacedialog.cpp
    replacedialog.h
    aboutdialog.cpp
    aboutdialog.h
)

target_link_libraries(TextEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 运行时把 Qt 动态库目录加入 PATH，便于直接双击运行
if(WIN32)
    set_target_properties(TextEditor PROPERTIES
        WIN32_EXECUTABLE TRUE
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
