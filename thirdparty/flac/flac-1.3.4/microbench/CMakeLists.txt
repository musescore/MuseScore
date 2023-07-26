if(MSVC)
    return()
endif()

set(CMAKE_REQUIRED_LIBRARIES rt)
check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)

if(APPLE)
    add_definitions(-DFLAC__SYS_DARWIN)
endif()

add_executable(benchmark_residual benchmark_residual.c util.c)
target_include_directories(benchmark_residual PRIVATE
    "$<TARGET_PROPERTY:FLAC,SOURCE_DIR>/include")
target_link_libraries(benchmark_residual
    FLAC
    $<$<BOOL:${HAVE_CLOCK_GETTIME}>:rt>)
