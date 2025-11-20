muse_create_thirdparty_module(beatroot)

target_sources(beatroot PRIVATE
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

target_no_warning(beatroot -Wno-conversion)
target_no_warning(beatroot -Wno-unused-but-set-variable)
