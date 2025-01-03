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

if (MSVC)
  set_target_properties( beatroot PROPERTIES
    COMPILE_FLAGS "/wd4267"
  )
elseif (APPLE)
  set_target_properties( beatroot PROPERTIES
    COMPILE_FLAGS "-Wno-shorten-64-to-32"
  )
endif ()
