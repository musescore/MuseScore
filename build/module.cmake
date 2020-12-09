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
# set(MODULE_QRC somename.qrc)  - set resource (qrc) file
# set(MODULE_UI ...)            - set ui headers
# set(MODULE_QML_IMPORT ...)    - set Qml import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_HAS_C_CODE ON)     - set if source contains C code

# After all the settings you need to do:
# include(${PROJECT_SOURCE_DIR}/build/module.cmake)

message(STATUS "Configuring " ${MODULE})

set(IS_DEBUG_BUILD OFF)
string(TOLOWER ${CMAKE_BUILD_TYPE} _build_type )
if (_build_type STREQUAL "debug")
   set(IS_DEBUG_BUILD ON)
endif()

set(LIBRARY_TYPE STATIC)
if (BUILD_SHARED_LIB_IN_DEBUG)
    if (IS_DEBUG_BUILD)
        include(GetCompilerInfo)
        if (COMPILER_IS_GCC)
            set(LIBRARY_TYPE SHARED)
        endif()
    endif(IS_DEBUG_BUILD)
endif(BUILD_SHARED_LIB_IN_DEBUG)

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

add_library(${MODULE} ${LIBRARY_TYPE})

if (${LIBRARY_TYPE} STREQUAL "SHARED")
    message(STATUS "  ${MODULE} is SHARED, will be installed to /bin")
    install(TARGETS ${MODULE} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

    if (NOT MSVC)
        set_target_properties(${MODULE} PROPERTIES COMPILE_FLAGS "-fPIC")
    endif (NOT MSVC)
endif()

target_sources(${MODULE} PRIVATE
    ${ui_headers}
    ${RCC_SOURCES}
    ${MODULE_SRC}
    )

target_include_directories(${MODULE} PUBLIC
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src/framework
    ${PROJECT_SOURCE_DIR}/src/framework/global
    ${PROJECT_SOURCE_DIR}/src
    ${MODULE_INCLUDE}
)

target_compile_definitions(${MODULE} PUBLIC
    ${MODULE_DEF}
    ${MODULE}_QML_IMPORT="${MODULE_QML_IMPORT}"
)

if(NOT ${MODULE} MATCHES global)
    set(MODULE_LINK global ${MODULE_LINK})
endif()

set(MODULE_LINK ${QT_LIBRARIES} ${MODULE_LINK})

target_link_libraries(${MODULE} PRIVATE ${MODULE_LINK} )

#set(LOCAL_MODULE_BUILD_PCH ${BUILD_PCH})

#if (MODULE_HAS_C_CODE)

#    set(LOCAL_MODULE_BUILD_PCH OFF)

#    if (NOT MSVC)
#        set_target_properties( ${MODULE} PROPERTIES COMPILE_FLAGS "-fPIC")
#    endif (NOT MSVC)

#endif(MODULE_HAS_C_CODE)


#if (LOCAL_MODULE_BUILD_PCH)
#    if (NOT MSVC)
#        set_target_properties (${MODULE} PROPERTIES COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch")
#    else (NOT MSVC)
#        set_target_properties (${MODULE} PROPERTIES COMPILE_FLAGS "${PCH_INCLUDE}" )
#    endif (NOT MSVC)

#    xcode_pch(${MODULE} all)

#    # Use MSVC pre-compiled headers
#    vstudio_pch( ${MODULE} )

#    # MSVC does not depend on mops1 & mops2 for PCH
#    if (NOT MSVC)
#        ADD_DEPENDENCIES(${MODULE} mops1)
#        ADD_DEPENDENCIES(${MODULE} mops2)
#    endif (NOT MSVC)
#else (LOCAL_MODULE_BUILD_PCH)

#    set(MODULE_USE_ALL_H OFF)

#    if (MODULE_USE_ALL_H)
#        if (NOT MSVC)
#            set_target_properties (${MODULE} PROPERTIES COMPILE_FLAGS "-include ${PROJECT_SOURCE_DIR}/all.h")
#        else (NOT MSVC)
#            set_target_properties (${MODULE} PROPERTIES COMPILE_FLAGS "/FI ${PROJECT_SOURCE_DIR}/all.h" )
#        endif (NOT MSVC)
#        message(STATUS "  ${MODULE} include all.h")
#    endif(MODULE_USE_ALL_H)

#endif (LOCAL_MODULE_BUILD_PCH)


