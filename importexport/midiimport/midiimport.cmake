

set (BEATROOT_SRC
    ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/Agent.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/AgentList.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/BeatTracker.cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/beatroot/Induction.cpp
    )


file(GLOB MIDIIMPORT_SRC
    ${CMAKE_CURRENT_LIST_DIR}/*.*
    ${BEATROOT_SRC}
    )
