
set (DRIVERS_DIR ${CMAKE_CURRENT_LIST_DIR})

set (DRIVERS_SRC
    ${DRIVERS_DIR}/driver.h
    ${DRIVERS_DIR}/driver.cpp
    )

if ( NOT MINGW AND NOT MSVC )
    if (USE_ALSA)
          set (DRIVERS_SRC ${DRIVERS_SRC} ${DRIVERS_DIR}/alsa.cpp ${DRIVERS_DIR}/alsa.h ${DRIVERS_DIR}/alsamidi.h)
    endif (USE_ALSA)
endif ( NOT MINGW AND NOT MSVC )

if (USE_PORTAUDIO)
    set (DRIVERS_SRC ${DRIVERS_SRC} ${DRIVERS_DIR}/pa.cpp ${DRIVERS_DIR}/pa.h)
endif (USE_PORTAUDIO)

if (USE_PULSEAUDIO)
    set (DRIVERS_SRC ${DRIVERS_SRC} ${DRIVERS_DIR}/pulseaudio.cpp)
endif (USE_PULSEAUDIO)

if (USE_PORTMIDI)
    set (DRIVERS_SRC ${DRIVERS_SRC} ${DRIVERS_DIR}/pm.cpp ${DRIVERS_DIR}/pm.h)
endif (USE_PORTMIDI)

if (USE_JACK)
      set (DRIVERS_SRC ${DRIVERS_SRC} ${DRIVERS_DIR}/jackaudio.cpp ${DRIVERS_DIR}/jackweakapi.cpp ${DRIVERS_DIR}/jackaudio.h)
endif (USE_JACK)

if (USE_ALSA OR USE_PORTMIDI)
      set (DRIVERS_SRC ${DRIVERS_SRC} ${DRIVERS_DIR}/mididriver.cpp ${DRIVERS_DIR}/mididriver.h)
endif (USE_ALSA OR USE_PORTMIDI)


