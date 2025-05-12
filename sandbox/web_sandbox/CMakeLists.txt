cmake_minimum_required(VERSION 3.16)

project(web_sandbox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(PROJECT_ROOT_DIR ${MU_ROOT})

set(CMAKE_MODULE_PATH
    ${MU_ROOT}/buildscripts/cmake
    ${CMAKE_MODULE_PATH}
)

include(SetupBuildEnvironment)
include(GetPlatformInfo)
include(DeclareModuleSetup)

# Qt
set(qt_components Core Gui Quick Qml Widgets)
set(qt_libs Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml Qt6::Widgets)
set(qt_includes )
if (OS_IS_LIN)
    list(APPEND qt_components DBus)
    list(APPEND qt_libs Qt::DBus)
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS ${qt_components})

foreach(qlib ${qt_libs} )
    get_target_property(Qt6X_includes ${qlib} INTERFACE_INCLUDE_DIRECTORIES)
    if (Qt6X_includes STREQUAL "Qt6X_includes-NOTFOUND")

    else()
        list(APPEND qt_includes ${Qt6X_includes})
    endif()
endforeach()

#set_target_properties(Qt6::Platform PROPERTIES INTERFACE_LINK_OPTIONS "${EMCC_LINKER_FLAGS}SHELL:-s LLD_REPORT_UNDEFINED;")

include_directories(appweb_sandbox PUBLIC
    ${qt_includes}
    ${MU_ROOT}/src/framework
    ${MU_ROOT}/src/framework/global
    ${MU_ROOT}/src/framework/actions
    ${MU_ROOT}/src/framework/ui
    ${MU_ROOT}/src
    ${MU_ROOT}/src/engraving
)

set(QT_SUPPORT ON)
set(GLOBAL_NO_INTERNAL OFF)
add_compile_definitions(QT_SUPPORT)
add_compile_definitions(KORS_LOGGER_QT_SUPPORT)
add_compile_definitions(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL)
add_compile_definitions(MUSE_MODULE_UI)

set(MUSE_APP_INSTALL_NAME "\"\"")
set(MUSE_APP_INSTALL_PREFIX "\"\"")

# framework
configure_file(${MU_ROOT}/src/framework/cmake/muse_framework_config.h.in muse_framework_config.h )
add_subdirectory(${MU_ROOT}/src/framework/global global)
add_subdirectory(${MU_ROOT}/src/framework/actions actions)
add_subdirectory(${MU_ROOT}/src/framework/ui ui)
add_subdirectory(${MU_ROOT}/src/framework/uicomponents uicomponents)

qt_add_resources(APP_RCC_SOURCES app.qrc)

qt_add_executable(appweb_sandbox
    ${APP_RCC_SOURCES}
    main.cpp
)

qt_add_qml_module(appweb_sandbox
    URI web_sandbox
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES interactivetestmodel.h interactivetestmodel.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appweb_sandbox PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appweb_sandbox
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appweb_sandbox
    PUBLIC ${qt_libs}
    muse::global
    muse::actions
    muse::ui
    muse::uicomponents
)

if (OS_IS_WASM)
    configure_file("${CMAKE_CURRENT_BINARY_DIR}/appweb_sandbox.html" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/appweb_sandbox.html" COPYONLY)
    configure_file("${CMAKE_CURRENT_BINARY_DIR}/qtloader.js" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtloader.js" COPYONLY)
    configure_file("${CMAKE_CURRENT_BINARY_DIR}/qtlogo.svg" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtlogo.svg" COPYONLY)
endif()

include(GNUInstallDirs)
install(TARGETS appweb_sandbox
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
