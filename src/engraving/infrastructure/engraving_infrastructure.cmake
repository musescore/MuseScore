
set (ENGRAVING_INFRASTRUCTURE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/draw/color.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/color.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/geometry.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/transform.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/transform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painterpath.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painterpath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/bezier.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/bezier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/drawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/pen.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/brush.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/pixmap.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/painter.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/ipaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/paintdevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/paintdevice.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/buffereddrawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/bufferedpaintprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/bufferedpaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/svgrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/svgrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/ifontprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/iimageprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/font.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontmetrics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontmetrics.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/rgba.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawlogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawlogger.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawjson.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawjson.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawcomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawcomp.h

    ${CMAKE_CURRENT_LIST_DIR}/interactive/messagebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/interactive/messagebox.h

    ${CMAKE_CURRENT_LIST_DIR}/io/mscio.h
    ${CMAKE_CURRENT_LIST_DIR}/io/mscreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/mscreader.h
    ${CMAKE_CURRENT_LIST_DIR}/io/mscwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/mscwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/io/htmlparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/htmlparser.h
    ${CMAKE_CURRENT_LIST_DIR}/io/ifileinfoprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/io/localfileinfoprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/localfileinfoprovider.h
)

set(ENGRAVING_INFRASTRUCTURE_DEF )
set(ENGRAVING_INFRASTRUCTURE_LINK )

if (NO_ENGRAVING_INTERNAL)
    set(ENGRAVING_INFRASTRUCTURE_DEF -DNO_ENGRAVING_INTERNAL)
else()
    set(ENGRAVING_INFRASTRUCTURE_SRC ${ENGRAVING_INFRASTRUCTURE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.h
        )

    if (USE_SYSTEM_FREETYPE)
         set(ENGRAVING_INFRASTRUCTURE_LINK freetype)
    else (USE_SYSTEM_FREETYPE)
         set(ENGRAVING_INFRASTRUCTURE_LINK mscore_freetype)
    endif (USE_SYSTEM_FREETYPE)

endif()
