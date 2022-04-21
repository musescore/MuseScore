add_executable(test_seeking
    main.c
    $<$<BOOL:${WIN32}>:../../include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:../share/win_utf8_io/win_utf8_io.c>)
target_link_libraries(test_seeking FLAC)
