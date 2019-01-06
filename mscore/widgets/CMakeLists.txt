set(WIDGETS_SOURCE_FILES
    filterabletreeview.cpp
    searchbox.cpp
    )

# expose variables to CMakeLists.txt in parent directory
set(WIDGETS_SOURCE_FILES "${WIDGETS_SOURCE_FILES}" PARENT_SCOPE)
