if(NOT ENABLE_GPL_CODE)
    message(FATAL_ERROR "Using the Linux build requires ENABLE_GPL_CODE to be true.")
endif()

set(crashreporter_SOURCES CrashReporter.cpp CrashReporterGzip.cpp)
set(crashreporter_UI CrashReporter.ui)
set(crashreporter_RC ../../resources.qrc)
set(crashreporter_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES})

if(UNIX AND NOT APPLE)
list(APPEND crashreporter_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../3rdparty/drkonqi-parser
)
list(APPEND crashreporter_LIBRARIES
    drkonqi_backtrace_parser
)
list(APPEND crashreporter_SOURCES
    linux-backtrace-generator/crashedapplication.cpp
    linux-backtrace-generator/backtracegenerator.cpp
    linux-backtrace-generator/debugger.cpp
    linux-backtrace-generator/crqt-kmacroexpander.cpp
    linux-backtrace-generator/crqt-kmacroexpander_unix.cpp
    linux-backtrace-generator/crqt-kshell.cpp
    linux-backtrace-generator/crqt-kshell_unix.cpp
)
endif()

add_library(crashreporter-gui STATIC
    ${crashreporter_SOURCES}
    ${crashreporter_HEADERS_MOC}
    ${crashreporter_UI_HEADERS}
    ${crashreporter_RC_RCC}
)

target_include_directories(crashreporter-gui PUBLIC ${crashreporter_INCLUDE_DIRECTORIES})

target_link_libraries(crashreporter-gui
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    ${crashreporter_LIBRARIES}
)

set_target_properties(crashreporter-gui PROPERTIES AUTOUIC ON)
set_target_properties(crashreporter-gui PROPERTIES AUTOMOC ON)
#install(TARGETS crashreporter-gui
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#)
