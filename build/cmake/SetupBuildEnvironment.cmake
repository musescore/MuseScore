
include(GetCompilerInfo)

if (COMPILER_IS_GCC)
    message(STATUS "Using Compiler GCC ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

elseif(COMPILER_IS_MSVC)
    message(STATUS "Using Compiler MSVC ${CMAKE_CXX_COMPILER_VERSION}")

    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)

elseif(COMPILER_IS_MINGW)
    message(STATUS "Using Compiler MINGW ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

    # -mno-ms-bitfields see #22048
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
    if (NOT BUILD_64)
        set(CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware")
    endif (NOT BUILD_64)

elseif(COMPILER_IS_CLANG)
    message(STATUS "Using Compiler CLANG ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

else()
    message(FATAL_ERROR "Unsupported Compiler CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
endif()


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
if (APPLE)
      set(CMAKE_OSX_ARCHITECTURES x86_64)
      set(MACOSX_DEPLOYMENT_TARGET 10.10)
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)
endif(APPLE)


#if (APPLE)

#else (APPLE)
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
