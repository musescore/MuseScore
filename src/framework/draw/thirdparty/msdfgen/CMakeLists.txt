set(MODULE msdfgen)
set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

set(MSDFGEN_DIR ${CMAKE_CURRENT_LIST_DIR}/msdfgen-1.4)

set(MODULE_SRC
    ${MSDFGEN_DIR}/core/Bitmap.cpp
    ${MSDFGEN_DIR}/core/Contour.cpp
    ${MSDFGEN_DIR}/core/Shape.cpp
    ${MSDFGEN_DIR}/core/SignedDistance.cpp
    ${MSDFGEN_DIR}/core/SignedDistance.h
    ${MSDFGEN_DIR}/core/Vector2.cpp
    ${MSDFGEN_DIR}/core/edge-segments.cpp
    ${MSDFGEN_DIR}/core/edge-segments.h
    ${MSDFGEN_DIR}/core/equation-solver.cpp
    ${MSDFGEN_DIR}/core/msdfgen.cpp
    ${MSDFGEN_DIR}/ext/import-font.cpp
    ${MSDFGEN_DIR}/msdfgen.h
    )

set(MODULE_INCLUDE
    ${MSDFGEN_DIR}/include
    )

set(MODULE_NOT_LINK_GLOBAL ON)
set(MODULE_PCH_DISABLED ON)
include(SetupModule)

target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-cast-function-type)
