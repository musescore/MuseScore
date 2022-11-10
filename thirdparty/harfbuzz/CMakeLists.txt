set(MODULE harfbuzz)
set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/harfbuzz)

set(MODULE_SRC
    ${MODULE_DIR}/src/harfbuzz.cc
)

set(MODULE_INCLUDE
    ${THIRD_PARTY}/freetype/include
)

set(MODULE_DEF
    -DHAVE_FREETYPE
)

set(MODULE_NOT_LINK_GLOBAL ON)
set(MODULE_PCH_DISABLED ON)
set(MODULE_UNITY_DISABLED ON)
include(SetupModule)

# target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
# target_no_warning(${MODULE} -Wno-conversion)
# target_no_warning(${MODULE} -Wno-cast-function-type)
