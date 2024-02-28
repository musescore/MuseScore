set(MODULE harfbuzz)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/src/harfbuzz.cc
)

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/../freetype/freetype-2.13.1/include
)

set(MODULE_DEF
    -DHAVE_FREETYPE
)

set(MODULE_NOT_LINK_GLOBAL ON)
set(MODULE_PCH_DISABLED ON)
set(MODULE_UNITY_DISABLED ON)
include(SetupModule)

target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-cast-function-type)
