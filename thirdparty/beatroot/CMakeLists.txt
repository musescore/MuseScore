
set(MODULE beatroot)

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

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


#if (APPLE)
#      file(GLOB_RECURSE INCS "*.h")
#else (APPLE)
#      set(INCS "")
#endif (APPLE)

#add_library (
#      beatroot STATIC
#      Induction.cpp
#      BeatTracker.cpp
#      AgentList.cpp
#      Agent.cpp
#  )

#if (NOT MSVC)
#   set_target_properties( beatroot
#      PROPERTIES
#         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
#      )
#else (NOT MSVC)
#   set_target_properties ( beatroot
#      PROPERTIES
#         COMPILE_FLAGS "${PCH_INCLUDE} /wd4267"
#      )
#endif (NOT MSVC)

#xcode_pch(beatroot all)

## Use MSVC pre-compiled headers
#vstudio_pch( beatroot )

## MSVC does not depend on mops1 & mops2 for PCH
#if (NOT MSVC)
#   ADD_DEPENDENCIES(beatroot mops1)
#   ADD_DEPENDENCIES(beatroot mops2)
#endif (NOT MSVC)

