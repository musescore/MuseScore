add_executable(flacdiff
    main.cpp
    $<$<BOOL:${WIN32}>:../../../include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:../../share/win_utf8_io/win_utf8_io.c>)
target_link_libraries(flacdiff FLAC++)
