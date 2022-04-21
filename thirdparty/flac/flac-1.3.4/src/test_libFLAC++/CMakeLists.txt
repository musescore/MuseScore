add_executable(test_libFLAC++
    decoders.cpp
    encoders.cpp
    main.cpp
    metadata.cpp
    metadata_manip.cpp
    metadata_object.cpp
    $<$<BOOL:${WIN32}>:../../include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:../share/win_utf8_io/win_utf8_io.c>)
target_link_libraries(test_libFLAC++ FLAC++ test_libs_common grabbag)
