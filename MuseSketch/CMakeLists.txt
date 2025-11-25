cmake_minimum_required(VERSION 3.16)

project(MuseSketch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Widgets Network Svg Xml Concurrent PrintSupport Multimedia)

# Add subdirectories
add_subdirectory(app)
add_subdirectory(core)
