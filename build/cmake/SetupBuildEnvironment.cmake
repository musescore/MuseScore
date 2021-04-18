
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

    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)

elseif(CC_IS_CLANG)
    message(STATUS "Using Compiler CLANG ${CMAKE_CXX_COMPILER_VERSION}")

    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")

elseif(CC_IS_EMSCRIPTEN)
    message(STATUS "Using Compiler Emscripten ${CMAKE_CXX_COMPILER_VERSION}")

    set(EMCC_CMAKE_TOOLCHAIN "" CACHE FILEPATH "Path to EMCC CMake Emscripten.cmake")
    set(EMCC_INCLUDE_PATH "." CACHE PATH "Path to EMCC include dir")
    set(EMCC_COMPILE_FLAGS "--bind -o .html --preload-file ../../files")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/public_html)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMCC_COMPILE_FLAGS}")
    set(CMAKE_TOOLCHAIN_FILE ${EMCC_CMAKE_TOOLCHAIN})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    #for QtCreator
    include_directories(AFTER
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
      set(MACOSX_DEPLOYMENT_TARGET 10.14)
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
endif(OS_IS_MAC)

