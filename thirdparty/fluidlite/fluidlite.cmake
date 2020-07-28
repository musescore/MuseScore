
set(FLUIDSYNTH_DIR ${CMAKE_CURRENT_LIST_DIR})

set(FLUIDSYNTH_INC ${FLUIDSYNTH_DIR}/include)

set(FLUIDSYNTH_SRC
        ${FLUIDSYNTH_DIR}/src/fluid_chan.c
        ${FLUIDSYNTH_DIR}/src/fluid_chorus.c
        ${FLUIDSYNTH_DIR}/src/fluid_conv.c
        ${FLUIDSYNTH_DIR}/src/fluid_defsfont.c
        ${FLUIDSYNTH_DIR}/src/fluid_dsp_float.c
        ${FLUIDSYNTH_DIR}/src/fluid_gen.c
        ${FLUIDSYNTH_DIR}/src/fluid_hash.c
        ${FLUIDSYNTH_DIR}/src/fluid_list.c
        ${FLUIDSYNTH_DIR}/src/fluid_mod.c
        ${FLUIDSYNTH_DIR}/src/fluid_ramsfont.c
        ${FLUIDSYNTH_DIR}/src/fluid_rev.c
        ${FLUIDSYNTH_DIR}/src/fluid_settings.c
        ${FLUIDSYNTH_DIR}/src/fluid_synth.c
        ${FLUIDSYNTH_DIR}/src/fluid_sys.c
        ${FLUIDSYNTH_DIR}/src/fluid_tuning.c
        ${FLUIDSYNTH_DIR}/src/fluid_voice.c
)
