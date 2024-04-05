
set(FLUIDSYNTH_DIR ${CMAKE_CURRENT_LIST_DIR}/fluidsynth-2.3.3)

set(FLUIDSYNTH_INC
    ${FLUIDSYNTH_DIR}/include
    ${FLUIDSYNTH_DIR}/src
    ${FLUIDSYNTH_DIR}/src/bindings
    ${FLUIDSYNTH_DIR}/src/drivers
    ${FLUIDSYNTH_DIR}/src/external
    ${FLUIDSYNTH_DIR}/src/midi
    ${FLUIDSYNTH_DIR}/src/rvoice
    ${FLUIDSYNTH_DIR}/src/sfloader
    ${FLUIDSYNTH_DIR}/src/synth
    ${FLUIDSYNTH_DIR}/src/utils
    )

set(FLUIDSYNTH_DEF
    -DNO_GLIB
    -DNO_THREADS
    )

set(FLUIDSYNTH_SRC
    # ${FLUIDSYNTH_DIR}/src/fluidsynth.c
    ${FLUIDSYNTH_DIR}/src/external/fluidsynthconfig.h
    ${FLUIDSYNTH_DIR}/src/external/fluid_conv_tables.inc.h
    ${FLUIDSYNTH_DIR}/src/external/fluid_rvoice_dsp_tables.inc.h
    ${FLUIDSYNTH_DIR}/src/external/portable_endian.h

    ${FLUIDSYNTH_DIR}/src/synth/fluid_chan.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_chan.h
    ${FLUIDSYNTH_DIR}/src/synth/fluid_event.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_event.h
    ${FLUIDSYNTH_DIR}/src/synth/fluid_gen.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_gen.h
    ${FLUIDSYNTH_DIR}/src/synth/fluid_mod.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_mod.h
    ${FLUIDSYNTH_DIR}/src/synth/fluid_synth.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_synth.h
    ${FLUIDSYNTH_DIR}/src/synth/fluid_synth_monopoly.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_tuning.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_tuning.h
    ${FLUIDSYNTH_DIR}/src/synth/fluid_voice.c
    ${FLUIDSYNTH_DIR}/src/synth/fluid_voice.h

    ${FLUIDSYNTH_DIR}/src/utils/fluid_conv.c
    ${FLUIDSYNTH_DIR}/src/utils/fluid_conv.h
    ${FLUIDSYNTH_DIR}/src/utils/fluid_conv_tables.h
    ${FLUIDSYNTH_DIR}/src/utils/fluid_hash.c
    ${FLUIDSYNTH_DIR}/src/utils/fluid_hash.h
    ${FLUIDSYNTH_DIR}/src/utils/fluid_list.c
    ${FLUIDSYNTH_DIR}/src/utils/fluid_list.h
    ${FLUIDSYNTH_DIR}/src/utils/fluid_ringbuffer.c
    ${FLUIDSYNTH_DIR}/src/utils/fluid_ringbuffer.h
    ${FLUIDSYNTH_DIR}/src/utils/fluid_settings.c
    ${FLUIDSYNTH_DIR}/src/utils/fluid_settings.h
    ${FLUIDSYNTH_DIR}/src/utils/fluidsynth_priv.h
    ${FLUIDSYNTH_DIR}/src/utils/fluid_sys.c
    ${FLUIDSYNTH_DIR}/src/utils/fluid_sys.h

    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_defsfont.c
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_defsfont.h
    # ${FLUIDSYNTH_DIR}/src/sfloader/fluid_instpatch.c
    # ${FLUIDSYNTH_DIR}/src/sfloader/fluid_instpatch.h
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_samplecache.c
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_samplecache.h
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_sffile.c
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_sffile.h
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_sfont.c
    ${FLUIDSYNTH_DIR}/src/sfloader/fluid_sfont.h

    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_adsr_env.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_adsr_env.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_chorus.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_chorus.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_iir_filter.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_iir_filter.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_lfo.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_lfo.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_phase.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rev.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rev.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice_dsp.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice_dsp_tables.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice_event.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice_event.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice.h
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice_mixer.c
    ${FLUIDSYNTH_DIR}/src/rvoice/fluid_rvoice_mixer.h

    ${FLUIDSYNTH_DIR}/src/midi/fluid_midi_router.c
    ${FLUIDSYNTH_DIR}/src/midi/fluid_midi_router.h
    ${FLUIDSYNTH_DIR}/src/midi/fluid_midi.c
    ${FLUIDSYNTH_DIR}/src/midi/fluid_midi.h
    ${FLUIDSYNTH_DIR}/src/midi/fluid_seq.c
    ${FLUIDSYNTH_DIR}/src/midi/fluid_seq_queue.cpp
    ${FLUIDSYNTH_DIR}/src/midi/fluid_seq_queue.h
    ${FLUIDSYNTH_DIR}/src/midi/fluid_seqbind.c
    ${FLUIDSYNTH_DIR}/src/midi/fluid_seqbind_notes.cpp
    ${FLUIDSYNTH_DIR}/src/midi/fluid_seqbind_notes.h

    ${FLUIDSYNTH_DIR}/src/bindings/fluid_filerenderer.c

    ${FLUIDSYNTH_DIR}/src/drivers/fluid_adriver.h
    ${FLUIDSYNTH_DIR}/src/drivers/fluid_adriver.c
    ${FLUIDSYNTH_DIR}/src/drivers/fluid_mdriver.h
    ${FLUIDSYNTH_DIR}/src/drivers/fluid_mdriver.c
    )

if (NOT MSVC)
   if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
   endif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
else (NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4013 /wd4100 /wd4101 /wd4115 /wd4127 /wd4201 /wd4244 /wd4267 /wd4456 /wd4701 /wd4703 /wd4706")
endif (NOT MSVC)
