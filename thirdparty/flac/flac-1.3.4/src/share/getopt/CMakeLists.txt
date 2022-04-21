check_include_file("string.h" HAVE_STRING_H)

find_package(Intl)

add_library(getopt STATIC getopt.c getopt1.c)

if(Intl_FOUND)
    target_include_directories(getopt PRIVATE ${Intl_INCLUDE_DIRS})
    target_link_libraries(getopt PUBLIC ${Intl_LIBRARIES})
    target_compile_definitions(getopt PRIVATE HAVE_LIBINTL_H)
endif()
