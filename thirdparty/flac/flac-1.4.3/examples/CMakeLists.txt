add_subdirectory("c/decode/file")
add_subdirectory("c/encode/file")

if(BUILD_CXXLIBS)
    add_subdirectory("cpp/decode/file")
    add_subdirectory("cpp/encode/file")
endif()
