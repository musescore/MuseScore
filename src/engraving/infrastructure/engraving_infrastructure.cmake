
set (ENGRAVING_INFRASTRUCTURE_SRC

    ${CMAKE_CURRENT_LIST_DIR}/interactive/messagebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/interactive/messagebox.h
    ${CMAKE_CURRENT_LIST_DIR}/interactive/imimedata.h

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

if (ENGRAVING_NO_INTERNAL)
    set(ENGRAVING_INFRASTRUCTURE_DEF -DENGRAVING_NO_INTERNAL)
else()
    set(ENGRAVING_INFRASTRUCTURE_SRC ${ENGRAVING_INFRASTRUCTURE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/engravingconfiguration.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qmimedataadapter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qmimedataadapter.h
        )
endif()
