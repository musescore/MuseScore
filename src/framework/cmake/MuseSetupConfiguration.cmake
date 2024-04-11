
macro(disable_module_deps name)
    if (NOT MUSE_MODULE_${name})
        set(MUSE_MODULE_${name}_TESTS OFF)
        set(MUSE_MODULE_${name}_API OFF)
    endif()
endmacro()

macro(disable_module_tests name)
    set(MUSE_MODULE_${name}_TESTS OFF)
endmacro()

include(${CMAKE_CURRENT_LIST_DIR}/MuseModules.cmake)

foreach(NAME ${MUSE_FRAMEWORK_MODULES})
    disable_module_deps(${NAME})
endforeach()

if (NOT MUSE_ENABLE_UNIT_TESTS)
    foreach(NAME ${MUSE_FRAMEWORK_MODULES})
        disable_module_tests(${NAME})
    endforeach()
endif()

# hard dependency
if (NOT MUSE_MODULE_AUDIO)
    set(MUSE_MODULE_MUSESAMPLER OFF)
    set(MUSE_MODULE_VST OFF)
endif()

if (NOT MUSE_MODULE_DIAGNOSTICS)
    set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT OFF)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/MuseFetchDependencies.cmake)

configure_file(${CMAKE_CURRENT_LIST_DIR}/muse_framework_config.h.in muse_framework_config.h )

include(DeclareModuleSetup)
