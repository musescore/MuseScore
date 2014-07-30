if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
else (APPLE)
      set(INCS "")
endif (APPLE)

add_library (
      beatroot STATIC
      Induction.cpp
      BeatTracker.cpp
      AgentList.cpp
      Agent.cpp
  )

set_target_properties( beatroot
   PROPERTIES
      COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
   )

xcode_pch(beatroot all)

ADD_DEPENDENCIES(beatroot mops1)
ADD_DEPENDENCIES(beatroot mops2)

