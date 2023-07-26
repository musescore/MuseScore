cmake_minimum_required(VERSION 3.12)

add_library(utf8 STATIC
    charset.c
    iconvert.c
    utf8.c)

target_link_libraries(utf8 PUBLIC grabbag $<TARGET_NAME_IF_EXISTS:Iconv::Iconv>)
