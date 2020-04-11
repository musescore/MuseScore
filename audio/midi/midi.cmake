

set (FLUID_DIR ${CMAKE_CURRENT_LIST_DIR}/fluid)
set (FLUID_SRC
    ${FLUID_DIR}/fluid.cpp
    ${FLUID_DIR}/fluid.h
    ${FLUID_DIR}/chan.cpp
    ${FLUID_DIR}/conv.cpp
    ${FLUID_DIR}/conv.h
    ${FLUID_DIR}/dsp.cpp
    ${FLUID_DIR}/fluidgui.cpp
    ${FLUID_DIR}/fluidgui.h
    ${FLUID_DIR}/gen.cpp
    ${FLUID_DIR}/gen.h
    ${FLUID_DIR}/mod.cpp
    ${FLUID_DIR}/sfont.cpp
    ${FLUID_DIR}/sfont.h
    ${FLUID_DIR}/sfont3.cpp
    ${FLUID_DIR}/voice.cpp
    ${FLUID_DIR}/voice.h
    )

set (FLUID_UI
    ${FLUID_DIR}/fluid_gui.ui
    )

set (ZERBERUS_SRC )
set (ZERBERUS_UI )
if (ZERBERUS)

    set (ZERBERUS_DIR ${CMAKE_CURRENT_LIST_DIR}/zerberus)
    set (ZERBERUS_SRC
        ${ZERBERUS_DIR}/zerberus.cpp
        ${ZERBERUS_DIR}/zerberus.h
        ${ZERBERUS_DIR}/channel.cpp
        ${ZERBERUS_DIR}/channel.h
        ${ZERBERUS_DIR}/filter.cpp
        ${ZERBERUS_DIR}/filter.h
        ${ZERBERUS_DIR}/instrument.cpp
        ${ZERBERUS_DIR}/instrument.h
        ${ZERBERUS_DIR}/sample.h
        ${ZERBERUS_DIR}/sfz.cpp
        ${ZERBERUS_DIR}/voice.cpp
        ${ZERBERUS_DIR}/voice.h
        ${ZERBERUS_DIR}/zerberusgui.cpp
        ${ZERBERUS_DIR}/zerberusgui.h
        ${ZERBERUS_DIR}/zone.cpp
        ${ZERBERUS_DIR}/zone.h
        )

    set (ZERBERUS_UI
        ${ZERBERUS_DIR}/zerberus_gui.ui
        )

endif (ZERBERUS)

set (MIDI_SRC

    ${FLUID_SRC}
    ${ZERBERUS_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/event.cpp
    ${CMAKE_CURRENT_LIST_DIR}/event.h
    ${CMAKE_CURRENT_LIST_DIR}/midifile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midifile.h
    ${CMAKE_CURRENT_LIST_DIR}/midiinstrument.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midiinstrument.h
    ${CMAKE_CURRENT_LIST_DIR}/midipatch.h
    ${CMAKE_CURRENT_LIST_DIR}/msynthesizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/msynthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/synthesizer.h
    ${CMAKE_CURRENT_LIST_DIR}/synthesizergui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/synthesizergui.h
    )

set (MIDI_UI
    ${FLUID_UI}
    ${ZERBERUS_UI}
    )

