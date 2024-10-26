# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sergio Martins <sergio.martins@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.15)

project(
    qtwidgets_layouting_example
    DESCRIPTION "KDDockWidgets standalone layouting library example using QtWidgets"
    HOMEPAGE_URL "https://github.com/KDAB/KDDockWidgets"
    LANGUAGES CXX C
)

if(NOT TARGET kddockwidgets)
    # For the purpose of our example, we're looking for Qt5 or Qt6 KDDW.
    # For your own purposes, just chose the one you need.
    find_package(KDDockWidgets QUIET)
    if(NOT KDDockWidgets_FOUND)
        find_package(KDDockWidgets-qt6 REQUIRED)
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIRS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For KDBindings
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rdparty/)

add_executable(qtwidgets_standalone_layouting main.cpp)
target_link_libraries(qtwidgets_standalone_layouting PRIVATE KDAB::kddockwidgets)

if(NOT TARGET nlohmann_json::nlohmann_json)
    include(../../../../nlohmann.cmake)
    link_to_nlohman(qtwidgets_standalone_layouting)
endif()
