cmake_minimum_required(VERSION 3.13)
project(consuming_test LANGUAGES C)

find_package(FluidSynth CONFIG REQUIRED)

add_executable(main main.c)

target_link_libraries(main PRIVATE FluidSynth::libfluidsynth)
