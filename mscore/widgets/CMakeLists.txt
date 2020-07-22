set(WIDGETS_SOURCE_FILES
    filterableview.cpp
    filterabletreeview.cpp
    searchbox.cpp
    )

set(WIDGETS_HEADER_FILES
    filterableview.h
    filterabletreeview.h
    searchbox.h
    )

# expose variables to CMakeLists.txt in parent directory
set(WIDGETS_SOURCE_FILES "${WIDGETS_SOURCE_FILES}" PARENT_SCOPE)

set(WIDGETS_HEADER_FILES "${WIDGETS_HEADER_FILES}" PARENT_SCOPE)
