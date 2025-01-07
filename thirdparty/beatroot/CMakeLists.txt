
declare_module(beatroot)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/Agent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Agent.h
    ${CMAKE_CURRENT_LIST_DIR}/AgentList.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AgentList.h
    ${CMAKE_CURRENT_LIST_DIR}/BeatTracker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BeatTracker.h
    ${CMAKE_CURRENT_LIST_DIR}/Event.h
    ${CMAKE_CURRENT_LIST_DIR}/Induction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Induction.h
    )

set(MODULE_USE_COVERAGE OFF)

setup_module()

target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-unused-but-set-variable)
