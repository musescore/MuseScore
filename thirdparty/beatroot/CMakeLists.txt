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

if (BUILD_PCH)
    target_link_libraries(beatroot global)
    target_precompile_headers(beatroot REUSE_FROM global)
endif (BUILD_PCH)
