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

if (OS_IS_WASM)
    qt_add_library(all_qml_plugins STATIC)
    qt_import_qml_plugins(all_qml_plugins)
    list(APPEND qt_libs all_qml_plugins)
endif()

# set(EMCC_LINKER_FLAGS "SHELL:-s ASYNCIFY;SHELL:-s USE_ZLIB=1;SHELL:-s USE_LIBPNG=1;SHELL:-s ENVIRONMENT=web;SHELL:-s WASM=1;SHELL:-s EXIT_RUNTIME=1;SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=1;SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=[UTF16ToString,stringToUTF16];SHELL:-s USE_WEBGL2=1;--bind;SHELL:-s FETCH=1;SHELL:-s INITIAL_MEMORY=32MB;SHELL:-s FULL_ES2=1;SHELL:-s DISABLE_EXCEPTION_CATCHING=1;SHELL:-s ALLOW_MEMORY_GROWTH=1;")

# # debug
# if (BUILD_IS_DEBUG)
#     set(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS}SHELL:-s DEMANGLE_SUPPORT=1;SHELL:-s GL_DEBUG=1;SHELL:-s ASSERTIONS=2;--profiling-funcs")
# # release
# else()
#     # 95.1 Mb
#     set(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS}-O2;-g0;")
#     # 94.3 Mb
#     #set(EMCC_LINKER_FLAGS "${EMCC_LINKER_FLAGS}-Oz;-g0;-flto;SHELL:--closure 0;SHELL:-s ASSERTIONS=0")
# endif()

#   INTERFACE_LINK_OPTIONS "SHELL:-s EXIT_RUNTIME=1;SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=1;SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=[UTF16ToString,stringToUTF16];SHELL:-s USE_WEBGL2=1;--bind;SHELL:-s FETCH=1;SHELL:-s INITIAL_MEMORY=20MB;SHELL:-s FULL_ES2=1;SHELL:-s DISABLE_EXCEPTION_CATCHING=1;SHELL:-s ALLOW_MEMORY_GROWTH=1;\$<\$<CONFIG:Debug>:;SHELL:-s DEMANGLE_SUPPORT=1;SHELL:-s GL_DEBUG=1;SHELL:-s ASSERTIONS=2;--profiling-funcs>"
#set(EMCC_LINKER_FLAGS "SHELL:-s EXIT_RUNTIME=1;SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=1;SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=[UTF16ToString,stringToUTF16];SHELL:-s USE_WEBGL2=1;--bind;SHELL:-s FETCH=1;SHELL:-s INITIAL_MEMORY=20MB;SHELL:-s FULL_ES2=1;SHELL:-s DISABLE_EXCEPTION_CATCHING=1;SHELL:-s ALLOW_MEMORY_GROWTH=1;\$<\$<CONFIG:Debug>:;SHELL:-s DEMANGLE_SUPPORT=1;SHELL:-s GL_DEBUG=1;SHELL:-s ASSERTIONS=2;--profiling-funcs>")

#set_target_properties(Qt6::Platform PROPERTIES INTERFACE_LINK_OPTIONS "${EMCC_LINKER_FLAGS}")

get_target_property(EMCC_LINKER_FLAGS Qt6::Platform INTERFACE_LINK_OPTIONS)
#message(STATUS "${EMCC_LINKER_FLAGS}")
set(EMCC_LINKER_FLAGS "SHELL:-s ASYNCIFY;${EMCC_LINKER_FLAGS}")
set_target_properties(Qt6::Platform PROPERTIES INTERFACE_LINK_OPTIONS "${EMCC_LINKER_FLAGS}")

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

target_link_options(appweb_sandbox PUBLIC "-s ASYNCIFY")

# if (OS_IS_WASM)
#     configure_file("${CMAKE_CURRENT_BINARY_DIR}/appweb_sandbox.html" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/appweb_sandbox.html" COPYONLY)
#     configure_file("${CMAKE_CURRENT_BINARY_DIR}/qtloader.js" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtloader.js" COPYONLY)
#     configure_file("${CMAKE_CURRENT_BINARY_DIR}/qtlogo.svg" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtlogo.svg" COPYONLY)
# endif()

include(GNUInstallDirs)
install(TARGETS appweb_sandbox
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
