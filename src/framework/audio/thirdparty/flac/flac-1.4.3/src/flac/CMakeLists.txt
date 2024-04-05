check_include_file("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_include_file("termios.h" HAVE_TERMIOS_H)

add_executable(flacapp
    analyze.c
    decode.c
    encode.c
    foreign_metadata.c
    main.c
    local_string_utils.c
    utils.c
    vorbiscomment.c
    version.rc
    $<$<BOOL:${WIN32}>:../../include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:../share/win_utf8_io/win_utf8_io.c>)
set_property(TARGET flacapp PROPERTY RUNTIME_OUTPUT_NAME flac)
set_property(TARGET flacapp PROPERTY PROJECT_LABEL "flac")
target_link_libraries(flacapp
    FLAC
    getopt
    replaygain_synthesis
    utf8)

install(TARGETS flacapp EXPORT targets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
