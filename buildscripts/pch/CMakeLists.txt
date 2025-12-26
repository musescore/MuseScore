message(STATUS "Configuring muse_pch")

add_library(muse_pch pch.cpp)

target_precompile_headers_clang_ccache(muse_pch PRIVATE pch.h)

target_link_libraries(muse_pch PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
)
