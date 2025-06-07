declare_thirdparty_module(fluidsynth)

include(${CMAKE_CURRENT_LIST_DIR}/fluidsynth.cmake)

set(MODULE_SRC
    ${FLUIDSYNTH_SRC}
    )

set (MODULE_INCLUDE
    ${FLUIDSYNTH_INC}
    )

set (MODULE_INCLUDE_PRIVATE
    ${SNDFILE_INCDIR}
    )

set (MODULE_DEF
    ${FLUIDSYNTH_DEF}
    )

if (LIBINSTPATCH_SUPPORT)
    list(APPEND MODULE_DEF -DLIBINSTPATCH_SUPPORT=1)
    list(APPEND MODULE_INCLUDE_PRIVATE ${GLIB_INCDIRS})
    list(APPEND MODULE_INCLUDE_PRIVATE ${GOBJECT_INCDIRS})
    list(APPEND MODULE_LINK ${GLIB_LIB})
    list(APPEND MODULE_LINK ${GOBJECT_LIB})
endif (LIBINSTPATCH_SUPPORT)

list(APPEND MODULE_LINK ${SNDFILE_LIB})

set(MODULE_USE_UNITY OFF)
set(MODULE_USE_COVERAGE OFF)

setup_module()

#target_no_warning(${MODULE} -Wunused-const-variable=0)
target_no_warning(${MODULE} -Wno-deprecated)
target_no_warning(${MODULE} -Wno-unused-parameter)
target_no_warning(${MODULE} -Wno-unused-variable)
target_no_warning(${MODULE} -Wno-type-limits)
target_no_warning(${MODULE} -Wno-unknown-pragmas)
target_no_warning(${MODULE} -Wno-conversion)
target_no_warning(${MODULE} -Wno-uninitialized)
target_no_warning(${MODULE} -Wno-deprecated-declarations)
target_no_warning(${MODULE} -WMSVC-no-translation-unit-is-empty)
target_no_warning(${MODULE} -WMSVC-no-nonstandard-extension-used)
target_no_warning(${MODULE} -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(${MODULE} -WMSVC-no-hides-previous)
target_no_warning(${MODULE} -WMSVC-no-undefined-assuming-extern)
target_no_warning(${MODULE} -WMSVC-named-type-definition-in-parentheses)
target_no_warning(${MODULE} -WMSVC-no-different-enum-types)
