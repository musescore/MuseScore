cmake_minimum_required(VERSION 3.5)

project(jsdoc_extractor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(jsdoc_extractor
    main.cpp
    utils.h
)

include(GNUInstallDirs)
install(TARGETS jsdoc_extractor
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
