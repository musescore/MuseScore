
set(API_V1_SRC
    ${CMAKE_CURRENT_LIST_DIR}/engravingapiv1.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingapiv1.h
    ${CMAKE_CURRENT_LIST_DIR}/qmlpluginapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qmlpluginapi.h
    ${CMAKE_CURRENT_LIST_DIR}/enums.cpp
    ${CMAKE_CURRENT_LIST_DIR}/enums.h
    ${CMAKE_CURRENT_LIST_DIR}/apitypes.h
    ${CMAKE_CURRENT_LIST_DIR}/apistructs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/apistructs.h
    ${CMAKE_CURRENT_LIST_DIR}/scoreelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scoreelement.h
    ${CMAKE_CURRENT_LIST_DIR}/elements.cpp
    ${CMAKE_CURRENT_LIST_DIR}/elements.h
    ${CMAKE_CURRENT_LIST_DIR}/score.cpp
    ${CMAKE_CURRENT_LIST_DIR}/score.h
    ${CMAKE_CURRENT_LIST_DIR}/style.cpp
    ${CMAKE_CURRENT_LIST_DIR}/style.h
    ${CMAKE_CURRENT_LIST_DIR}/excerpt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/excerpt.h
    ${CMAKE_CURRENT_LIST_DIR}/cursor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cursor.h
    ${CMAKE_CURRENT_LIST_DIR}/part.cpp
    ${CMAKE_CURRENT_LIST_DIR}/part.h
    ${CMAKE_CURRENT_LIST_DIR}/instrument.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrument.h
    ${CMAKE_CURRENT_LIST_DIR}/playevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playevent.h
    ${CMAKE_CURRENT_LIST_DIR}/selection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/selection.h
)

# Disable unity build to avoid ambiguity errors with api vs dom classes
set_source_files_properties(
    ${API_V1_SRC}
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)
