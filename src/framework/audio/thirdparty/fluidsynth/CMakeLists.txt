muse_create_thirdparty_module(fluidsynth)

include(${CMAKE_CURRENT_LIST_DIR}/fluidsynth.cmake)

target_sources(fluidsynth PRIVATE ${FLUIDSYNTH_SRC})

target_include_directories(fluidsynth PUBLIC ${FLUIDSYNTH_INC})

target_compile_definitions(fluidsynth PUBLIC ${FLUIDSYNTH_DEF})

if (NOT MUSE_ENABLE_UNIT_TESTS_CODE_COVERAGE)
    target_compile_definitions(fluidsynth PRIVATE EXTERN_VORBIS_SUPPORT)
endif()

#target_no_warning(fluidsynth -Wunused-const-variable=0)
target_no_warning(fluidsynth -Wno-deprecated)
target_no_warning(fluidsynth -Wno-unused-parameter)
target_no_warning(fluidsynth -Wno-unused-variable)
target_no_warning(fluidsynth -Wno-type-limits)
target_no_warning(fluidsynth -Wno-unknown-pragmas)
target_no_warning(fluidsynth -Wno-conversion)
target_no_warning(fluidsynth -Wno-uninitialized)
target_no_warning(fluidsynth -Wno-deprecated-declarations)
target_no_warning(fluidsynth -WMSVC-no-translation-unit-is-empty)
target_no_warning(fluidsynth -WMSVC-no-nonstandard-extension-used)
target_no_warning(fluidsynth -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(fluidsynth -WMSVC-no-hides-previous)
target_no_warning(fluidsynth -WMSVC-no-undefined-assuming-extern)
target_no_warning(fluidsynth -WMSVC-named-type-definition-in-parentheses)
target_no_warning(fluidsynth -WMSVC-no-different-enum-types)
