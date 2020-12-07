


if (MINGW)
      set (ICON_RES_FILE ${PROJECT_BINARY_DIR}/resfile.o)
endif (MINGW)
if (MSVC)
      # MSVC recognizes a *.rc file and will invoke the resource compiler to link it
      set (ICON_RES_FILE ${CMAKE_CURRENT_LIST_DIR}/res/mscore.rc)
endif ( MSVC )
