include("icon-functions.cmake")

set(WINDOWS_ICO_SIZES 16 24 32 48 64 256) # https://docs.microsoft.com/en-gb/windows/desktop/uxguide/vis-icons
set(MACOS_ICNS_SIZES 16 32 128 256 512 1024) # https://stackoverflow.com/a/11788723
set(FREEDESKTOP_COMMON_SIZES 16 22 24 32 48 64 96 128 256 512)

set(ALL_ICON_SIZES ${WINDOWS_ICO_SIZES} ${MACOS_ICNS_SIZES} ${FREEDESKTOP_COMMON_SIZES})

list(REMOVE_DUPLICATES ALL_ICON_SIZES)
sort_integer_list(ALL_ICON_SIZES)

add_subdirectory(app)
add_subdirectory(file)