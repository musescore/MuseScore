

set (RTF2HTML_SRC
    ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/fmt_opts.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/rtf2html.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/rtf_keyword.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/rtf2html/rtf_table.cpp
    )

file(GLOB CAPELLA_SRC
    ${CMAKE_CURRENT_LIST_DIR}/*.*
    ${RTF2HTML_SRC}
    )
