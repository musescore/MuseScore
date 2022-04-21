add_library(xmms-flac STATIC
    charset.c
    configure.c
    fileinfo.c
    http.c
    plugin.c
    tag.c)
target_link_libraries(xmms-flac plugin_common)
