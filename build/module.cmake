#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

## Setup
# set(MODULE somename)          - set module (target) name
# set(MODULE_INCLUDE ...)       - set include (by default see below include_directories)
# set(MODULE_DEF ...)           - set definitions
# set(MODULE_SRC ...)           - set sources and headers files
# set(MODULE_LINK ...)          - set libraries for link
# set(MODULE_NOT_LINK_GLOBAL ON) - set for not link global lib
# set(MODULE_QRC somename.qrc)  - set resource (qrc) file
# set(MODULE_UI ...)            - set ui headers
# set(MODULE_QML_IMPORT ...)    - set Qml import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_USE_PCH_NONE ON)   - set for disable PCH for module
# set(MODULE_USE_UNITY_NONE ON) - set for disable UNITY BUILD for module
# set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR}) - set root dir for module

# After all the settings you need to do:
# include(${PROJECT_SOURCE_DIR}/build/module.cmake)

message(STATUS "Configuring " ${MODULE})

if (NOT PROJECT_ROOT_DIR)
    set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR})
endif()

if (MODULE_QRC)
    qt5_add_resources(RCC_SOURCES ${MODULE_QRC})
endif()

if (MODULE_UI)
    find_package(Qt5Widgets)
    QT5_WRAP_UI(ui_headers ${MODULE_UI} )
endif()

if (NOT ${MODULE_QML_IMPORT} STREQUAL "")
    set(QML_IMPORT_PATH "${QML_IMPORT_PATH};${MODULE_QML_IMPORT}" CACHE STRING "QtCreator extra import paths for QML modules" FORCE)
endif()

if (CC_IS_EMSCRIPTEN)
    add_library(${MODULE} OBJECT)
else()
    add_library(${MODULE}) # STATIC/SHARED set global in the SetupBuildEnvironment.cmake
endif()

if (BUILD_SHARED_LIBS)
    install(TARGETS ${MODULE} DESTINATION ${SHARED_LIBS_INSTALL_DESTINATION})

    if (NOT MSVC)
        set_target_properties(${MODULE} PROPERTIES COMPILE_FLAGS "-fPIC")
    endif (NOT MSVC)
endif()

if (BUILD_PCH)
    if (MODULE_USE_PCH_NONE)
        # disabled pch for current module
    else()
        if (NOT ${MODULE} MATCHES global)
            target_precompile_headers(${MODULE} REUSE_FROM global)
            target_compile_definitions(${MODULE} PRIVATE global_EXPORTS=1)
            if (MODULE_NOT_LINK_GLOBAL)
                set(MODULE_NOT_LINK_GLOBAL OFF)
            endif()
        else()
            target_precompile_headers(${MODULE} PRIVATE ${PROJECT_SOURCE_DIR}/build/pch/pch.h)
        endif()
    endif()
endif(BUILD_PCH)

if (BUILD_UNITY)
    if (MODULE_USE_UNITY_NONE)
        # disabled unity build for current module
        set_target_properties(${MODULE} PROPERTIES UNITY_BUILD OFF)
    else()
        set_target_properties(${MODULE} PROPERTIES UNITY_BUILD ON)
    endif()
endif(BUILD_UNITY)

target_sources(${MODULE} PRIVATE
    ${ui_headers}
    ${RCC_SOURCES}
    ${MODULE_SRC}
    )

target_include_directories(${MODULE} PUBLIC
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_ROOT_DIR}
    ${PROJECT_ROOT_DIR}/src
    ${PROJECT_ROOT_DIR}/src/framework
    ${PROJECT_ROOT_DIR}/src/framework/global
    ${PROJECT_ROOT_DIR}/src/engraving
    ${PROJECT_ROOT_DIR}/thirdparty/googletest/googletest/include
    ${MODULE_INCLUDE}
)

target_compile_definitions(${MODULE} PUBLIC
    ${MODULE_DEF}
    PROJECT_ROOT_DIR="${PROJECT_ROOT_DIR}"
    ${MODULE}_QML_IMPORT="${MODULE_QML_IMPORT}"
)

if (NOT ${MODULE} MATCHES global)
    if (NOT MODULE_NOT_LINK_GLOBAL)
        set(MODULE_LINK global ${MODULE_LINK})
    endif()
endif()

set(MODULE_LINK ${QT_LIBRARIES} ${MODULE_LINK})

target_link_libraries(${MODULE} PRIVATE ${MODULE_LINK} )
