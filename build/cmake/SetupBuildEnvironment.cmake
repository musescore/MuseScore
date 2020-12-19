
include(GetCompilerInfo)
include(GetBuildType)

set(BUILD_SHARED_LIBS OFF)
set(SHARED_LIBS_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

set(CMAKE_UNITY_BUILD_BATCH_SIZE 12)

if (CC_IS_GCC)
    message(STATUS "Using Compiler GCC ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

    if (TRY_BUILD_SHARED_LIBS_IN_DEBUG AND BUILD_IS_DEBUG)
        set(BUILD_SHARED_LIBS ON)
    endif()

elseif(CC_IS_MSVC)
    message(STATUS "Using Compiler MSVC ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS                 "/MP /EHsc /execution-charset:utf-8 /source-charset:utf-8")
    set(CMAKE_C_FLAGS                   "/MP /execution-charset:utf-8 /source-charset:utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG           "/MT /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE         "/MT /O2 /Ob2")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "/MT /Zi /O2 /Ob1")
    set(CMAKE_C_FLAGS_DEBUG             "/MT /Zi /Ob0 /Od /RTC1")
    set(CMAKE_C_FLAGS_RELEASE           "/MT /O2 /Ob2")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO    "/MT /Zi /O2 /Ob1")
    set(CMAKE_EXE_LINKER_FLAGS          "/DYNAMICBASE:NO")

    add_definitions(-DWIN32)
    add_definitions(-D_WINDOWS)
    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)

elseif(CC_IS_MINGW)
    message(STATUS "Using Compiler MINGW ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

    if (TRY_BUILD_SHARED_LIBS_IN_DEBUG AND BUILD_IS_DEBUG)
        set(BUILD_SHARED_LIBS ON)
    endif()

    # -mno-ms-bitfields see #22048
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
    if (NOT BUILD_64)
        set(CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware")
    endif (NOT BUILD_64)

elseif(CC_IS_CLANG)
    message(STATUS "Using Compiler CLANG ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

elseif(CC_IS_EMSCRIPTEN)
    message(STATUS "Using Compiler Emscripten ${CMAKE_CXX_COMPILER_VERSION}")

    set(EMCC_CMAKE_TOOLCHAIN "" CACHE FILEPATH "Path to EMCC CMake Emscripten.cmake")
    set(EMCC_INCLUDE_PATH "." CACHE PATH "Path to EMCC include dir")
    set(EMCC_COMPILE_FLAGS "--bind -o  .html")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/public_html)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMCC_COMPILE_FLAGS}")
    set(CMAKE_TOOLCHAIN_FILE ${EMCC_CMAKE_TOOLCHAIN})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    #for QtCreator
    include_directories(
        ${EMCC_INCLUDE_PATH}
        ${EMCC_INCLUDE_PATH}/libcxx
        ${EMCC_INCLUDE_PATH}/libc
    )

else()
    message(FATAL_ERROR "Unsupported Compiler CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
endif()

##
# Setup compile warnings
##
include(SetupCompileWarnings)

# Common
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE MATCHES "DEBUG") #Debug

    add_definitions(-DQT_QML_DEBUG)

else() #Release

    add_definitions(-DNDEBUG)
    add_definitions(-DQT_NO_DEBUG)
    add_definitions(-DQT_NO_DEBUG_OUTPUT)

endif()


# APPLE specific
if (OS_IS_MAC)
      set(CMAKE_OSX_ARCHITECTURES x86_64)
      set(MACOSX_DEPLOYMENT_TARGET 10.10)
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)
endif(OS_IS_MAC)


#   string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
#   # Windows: Add -mconsole to LINK_FLAGS to get a console window for debug output
#   if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
#     set_target_properties( mscore
#        PROPERTIES
#           COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
#           LINK_FLAGS "-mwindows -mconsole -L ${QT_INSTALL_LIBS}"
#        )
#   else(CMAKE_BUILD_TYPE MATCHES "DEBUG")
#     set_target_properties( mscore
#          PROPERTIES
#             COMPILE_FLAGS "${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
#             LINK_FLAGS "-Wl,-S -mwindows -L ${QT_INSTALL_LIBS}"
#          )
#   endif(CMAKE_BUILD_TYPE MATCHES "DEBUG")


#if (OS_IS_MAC)

#else (OS_IS_MAC)
#   if (MSVC)
#   # Set compiler options for VS2017/19 toolchain.
#   # Note: /D_CRT_SECURE_NO WARNINGS disables warnings when using "non-secure" library functions like sscanf...
##      set(CMAKE_CXX_FLAGS                "/MP /DWIN32 /D_WINDOWS /GR /EHsc /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS /execution-charset:utf-8 /source-charset:utf-8")
##      set(CMAKE_C_FLAGS                  "/MP /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS")
##      set(CMAKE_CXX_FLAGS_DEBUG          "/MT /permissive- /std:c++17 /W4 /Zi /Ob0 /Od /RTC1")
##      set(CMAKE_CXX_FLAGS_RELEASE        "/MT /permissive- /std:c++17 /W4 /O2 /Ob2 /DNDEBUG /DQT_NO_DEBUG /DQT_NO_DEBUG_OUTPUT")
##      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /permissive- /std:c++17 /W4 /Zi /O2 /Ob1 /DNDEBUG /DQT_NO_DEBUG /DQT_NO_DEBUG_OUTPUT")
##      set(CMAKE_C_FLAGS_DEBUG            "/MT /W4 /Zi /Ob0 /Od /RTC1")
##      set(CMAKE_C_FLAGS_RELEASE          "/MT /W4 /O2 /Ob2 /DNDEBUG")
##      set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/MT /W4 /Zi /O2 /Ob1 /DNDEBUG")
##      set(CMAKE_EXE_LINKER_FLAGS         "/DYNAMICBASE:NO")

#endif(APPLE)

#set(CMAKE_BUILD_WITH_INSTALL_RPATH ON) # Call CMake with option -DCMAKE_SKIP_RPATH to not set RPATH (Debian packaging requirement)
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
