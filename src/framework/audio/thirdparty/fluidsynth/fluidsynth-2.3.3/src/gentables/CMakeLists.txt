cmake_minimum_required(VERSION 3.1)

# remove $CC from the current environment and by that force cmake to look for a (working) C compiler,
# which hopefully will be the host compiler
unset(ENV{CC})

# also unset $CFLAGS to avoid passing any cross compilation flags to the host compiler
unset(ENV{CFLAGS})

# linker flags as well
unset(ENV{LDFLAGS})

project (gentables C)

set ( CMAKE_BUILD_TYPE Debug )

# hardcode ".exe" as suffix to the binary, else in case of cross-platform cross-compiling the calling cmake will not know the suffix used here and fail to find the binary
set ( CMAKE_EXECUTABLE_SUFFIX ".exe" )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

# Add the executable that generates the table
add_executable( make_tables
                make_tables.c
                gen_conv.c
                gen_rvoice_dsp.c)

target_include_directories( make_tables PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ )

if ( WIN32 )
    add_definitions ( -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS )
else ( WIN32 )
    target_link_libraries (make_tables "m")
endif ()
