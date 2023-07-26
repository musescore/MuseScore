add_library(test_libs_common STATIC
    file_utils_flac.c
    metadata_utils.c)
target_link_libraries(test_libs_common PUBLIC FLAC)
