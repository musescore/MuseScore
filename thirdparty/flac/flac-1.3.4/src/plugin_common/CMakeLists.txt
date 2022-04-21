cmake_minimum_required(VERSION 3.12)

add_library(plugin_common STATIC
    charset.c
    dither.c
    replaygain.c
    tags.c)
target_link_libraries(plugin_common PUBLIC $<TARGET_NAME_IF_EXISTS:Iconv::Iconv>)
