# This file is part of KDDockWidgets.
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sérgio Martins <sergio.martins@kdab.com>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
# Contact KDAB at <info@kdab.com> for commercial licensing options.

cmake_minimum_required(VERSION 3.21)
project(slint_standalone_layouting LANGUAGES CXX)

find_package(Slint QUIET)
if(NOT Slint_FOUND)
    message(
        "Slint could not be located in the CMake module search path. Downloading it from Git and building it locally"
    )
    include(FetchContent)
    fetchcontent_declare(
        Slint
        GIT_REPOSITORY https://github.com/slint-ui/slint.git
        GIT_TAG release/1
        SOURCE_SUBDIR api/cpp
    )
    fetchcontent_makeavailable(Slint)
endif(NOT Slint_FOUND)

add_executable(slint_standalone_layouting src/main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../3rdparty/)
include_directories(${CMAKE_BINARY_DIR})

target_link_libraries(slint_standalone_layouting PRIVATE Slint::Slint KDAB::kddockwidgets)
target_compile_definitions(slint_standalone_layouting PRIVATE SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

slint_target_sources(slint_standalone_layouting ui/appwindow.slint)
