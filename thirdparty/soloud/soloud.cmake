

set(SOLOUD_DIR ${CMAKE_CURRENT_LIST_DIR})

set(SOLOUD_INC
    ${SOLOUD_DIR}/include
)

set(SOLOUD_DEF
    -DWITH_MUAUDIO
    -DSOLOUD_NO_FLAC
    -DSOLOUD_USE_DECODER_SEEK
)

set(SOLOUD_SRC
    ${SOLOUD_DIR}/src/core/soloud_audiosource.cpp
    ${SOLOUD_DIR}/src/core/soloud_bus.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_3d.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_basicops.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_faderops.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_filterops.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_getters.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_setters.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_voicegroup.cpp
    ${SOLOUD_DIR}/src/core/soloud_core_voiceops.cpp
    ${SOLOUD_DIR}/src/core/soloud.cpp
    ${SOLOUD_DIR}/src/core/soloud_fader.cpp
    ${SOLOUD_DIR}/src/core/soloud_fft.cpp
    ${SOLOUD_DIR}/src/core/soloud_fft_lut.cpp
    ${SOLOUD_DIR}/src/core/soloud_file.cpp
    ${SOLOUD_DIR}/src/core/soloud_filter.cpp
    ${SOLOUD_DIR}/src/core/soloud_queue.cpp
    ${SOLOUD_DIR}/src/core/soloud_thread.cpp
    ${SOLOUD_DIR}/src/audiosource/wav/dr_impl.cpp
    #${SOLOUD_DIR}/src/audiosource/wav/stb_vorbis.c
    ${SOLOUD_DIR}/src/audiosource/wav/soloud_wavstream.cpp
    ${SOLOUD_DIR}/src/backend/muaudio/soloud_muaudio.cpp
)
