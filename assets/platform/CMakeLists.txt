include("icon-functions.cmake")

if(BUILD_WINDOWS_ICONS)
  # https://docs.microsoft.com/en-gb/windows/desktop/uxguide/vis-icons
  set(WINDOWS_ICO_SIZES 16 24 32 48 64 96 128 256)
  assert_ascending(WINDOWS_ICO_SIZES) # keep ordered for easy maintainability
else(BUILD_WINDOWS_ICONS)
  set(WINDOWS_ICO_SIZES "") # empty list ("unset" exposes cached values)
endif(BUILD_WINDOWS_ICONS)

if(BUILD_MACOS_ICONS)
  # https://stackoverflow.com/a/11788723
  set(MACOS_ICNS_SIZES 16 32 128 256 512) # sizes in "points" not pixels
  assert_ascending(MACOS_ICNS_SIZES)
else(BUILD_MACOS_ICONS)
  set(MACOS_ICNS_SIZES "")
endif(BUILD_MACOS_ICONS)

if(BUILD_FREEDESKTOP_ICONS)
  # Look in /usr/share/icons/hicolor and find subdirectories with most files
  set(FREEDESKTOP_COMMON_SIZES 16 22 24 32 48 64 96 128 256 512)
  assert_ascending(FREEDESKTOP_COMMON_SIZES)
else(BUILD_FREEDESKTOP_ICONS)
  set(FREEDESKTOP_COMMON_SIZES "")
endif(BUILD_FREEDESKTOP_ICONS)

set(ALL_ICON_SIZES ${WINDOWS_ICO_SIZES} ${MACOS_ICNS_SIZES} ${FREEDESKTOP_COMMON_SIZES})

list(REMOVE_DUPLICATES ALL_ICON_SIZES)
sort_integer_list(ALL_ICON_SIZES)

add_subdirectory(app-icons)
add_subdirectory(file-icons)
