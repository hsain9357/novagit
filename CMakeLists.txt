cmake_minimum_required(VERSION 3.16)
project(GitGUIApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core REQUIRED)

add_executable(GitGUIApp
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/GitManager.cpp
    src/GitManager.h
    src/DiffView.cpp
    src/DiffView.h
)

target_link_libraries(GitGUIApp PRIVATE Qt6::Widgets Qt6::Core)
