add_executable(test_libFLAC
    bitreader.c
    bitwriter.c
    crc.c
    decoders.c
    encoders.c
    endswap.c
    format.c
    main.c
    metadata.c
    metadata_manip.c
    metadata_object.c
    md5.c
    "$<TARGET_PROPERTY:FLAC,SOURCE_DIR>/bitreader.c"
    "$<TARGET_PROPERTY:FLAC,SOURCE_DIR>/bitwriter.c"
    "$<TARGET_PROPERTY:FLAC,SOURCE_DIR>/crc.c"
    "$<TARGET_PROPERTY:FLAC,SOURCE_DIR>/md5.c"
    $<$<BOOL:${WIN32}>:../../include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:../share/win_utf8_io/win_utf8_io.c>)

target_compile_definitions(test_libFLAC PRIVATE
    $<$<BOOL:${ENABLE_64_BIT_WORDS}>:ENABLE_64_BIT_WORDS>)
target_include_directories(test_libFLAC PRIVATE
    "$<TARGET_PROPERTY:FLAC,SOURCE_DIR>/include")
target_link_libraries(test_libFLAC FLAC grabbag test_libs_common)
