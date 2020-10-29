list(APPEND drkonqi_backtrace_parser_INCLUDE_DIRECTORIES
    ${QT_QTCORE_INCLUDE_DIR}
)


set(BACKTRACEPARSER_SRCS
    backtraceparser.cpp
    backtraceparsergdb.cpp
    backtraceparserkdbgwin.cpp
    backtraceparsernull.cpp
)


include_directories(${drkonqi_backtrace_parser_INCLUDE_DIRECTORIES})
add_library(drkonqi_backtrace_parser STATIC ${BACKTRACEPARSER_SRCS})
target_link_libraries(drkonqi_backtrace_parser
    PUBLIC
        Qt5::Core
)
set_target_properties(drkonqi_backtrace_parser PROPERTIES AUTOMOC ON)
