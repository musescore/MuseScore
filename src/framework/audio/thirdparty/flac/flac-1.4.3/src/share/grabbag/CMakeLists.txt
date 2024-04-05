add_library(grabbag STATIC
    alloc.c
    cuesheet.c
    file.c
    picture.c
    replaygain.c
    seektable.c
    snprintf.c)
target_link_libraries(grabbag PUBLIC
    FLAC
    replaygain_analysis)
if(TARGET win_utf8_io)
    target_link_libraries(grabbag PUBLIC win_utf8_io)
endif()
