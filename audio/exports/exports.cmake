


set (EXPORTS_SRC
    ${CMAKE_CURRENT_LIST_DIR}/exportmidi.h
    ${CMAKE_CURRENT_LIST_DIR}/exportmidi.cpp
    )

if (USE_LAME)
      set (EXPORTS_SRC ${EXPORTS_SRC} ${CMAKE_CURRENT_LIST_DIR}/exportmp3.cpp ${CMAKE_CURRENT_LIST_DIR}/exportmp3.h)
endif (USE_LAME)
