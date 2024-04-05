add_executable(metaflac
    main.c
    operations.c
    operations_shorthand_cuesheet.c
    operations_shorthand_picture.c
    operations_shorthand_seektable.c
    operations_shorthand_streaminfo.c
    operations_shorthand_vorbiscomment.c
    options.c
    usage.c
    utils.c
    version.rc
    $<$<BOOL:${WIN32}>:../../include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:../share/win_utf8_io/win_utf8_io.c>)
target_link_libraries(metaflac FLAC getopt utf8)

install(TARGETS metaflac EXPORT targets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
