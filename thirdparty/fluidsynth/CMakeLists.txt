
set(MODULE fluidsynth)

include(${CMAKE_CURRENT_LIST_DIR}/fluidsynth.cmake)

set(MODULE_SRC
    ${FLUIDSYNTH_SRC}
    )

include(FindSndFile)

set (MODULE_INCLUDE
    ${FLUIDSYNTH_INC}
    ${SNDFILE_INCDIR}
    )

set (MODULE_DEF
    ${FLUIDSYNTH_DEF}
    )

set(MODULE_LINK
    ${SNDFILE_LIB}
    )

set(MODULE_HAS_C_CODE 1)
include(${PROJECT_SOURCE_DIR}/build/module.cmake)

target_no_warning(${MODULE} -Wunused-const-variable=0)
target_no_warning(${MODULE} -Wno-unused-parameter)
target_no_warning(${MODULE} -Wno-unused-variable)
target_no_warning(${MODULE} -Wno-type-limits)
target_no_warning(${MODULE} -Wno-unknown-pragmas)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-uninitialized)
target_no_warning(${MODULE} -WMSVC-no-translation-unit-is-empty)
target_no_warning(${MODULE} -WMSVC-no-nonstandard-extension-used)
target_no_warning(${MODULE} -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(${MODULE} -WMSVC-no-hides-previous)
target_no_warning(${MODULE} -WMSVC-no-undefined-assuming-extern)
target_no_warning(${MODULE} -WMSVC-named-type-definition-in-parentheses)
