if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
else (APPLE)
      set(INCS "")
endif (APPLE)

add_library (beatroot STATIC
      Induction.cpp
      BeatTracker.cpp
      AgentList.cpp
      Agent.cpp
  )
