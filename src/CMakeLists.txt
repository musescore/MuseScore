
# Main
add_subdirectory(main)

# Framework
add_subdirectory(framework)

# Domain model
add_subdirectory(libmscore)

# AppShell
add_subdirectory(appshell)

# Modules
add_subdirectory(context)
add_subdirectory(notation)

if (BUILD_USERSCORES_MODULE)
    add_subdirectory(userscores)
endif (BUILD_USERSCORES_MODULE)

add_subdirectory(converter)

if (BUILD_WORKSPACE_MODULE)
    add_subdirectory(workspace)
endif (BUILD_WORKSPACE_MODULE)

if (NOT OS_IS_WASM)
    add_subdirectory(importexport)

    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/beatroot beatroot) # for midiimport
    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rtf2html rtf2html) # for capella

    if (BUILD_CLOUD_MODULE)
        add_subdirectory(cloud)
    endif (BUILD_CLOUD_MODULE)

    if (BUILD_EXTENSIONS_MODULE)
        add_subdirectory(extensions)
    endif (BUILD_EXTENSIONS_MODULE)

    if (BUILD_LANGUAGES_MODULE)
        add_subdirectory(languages)
    endif (BUILD_LANGUAGES_MODULE)

    if (BUILD_PLUGINS_MODULE)
        add_subdirectory(plugins)
    endif (BUILD_PLUGINS_MODULE)
endif()

# Scenes common
add_subdirectory(commonscene)
if (BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif (BUILD_PLAYBACK_MODULE)

# Notation
if (BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif (BUILD_PALETTE_MODULE)
if (BUILD_INSTRUMENTS_MODULE)
    add_subdirectory(instruments)
endif (BUILD_INSTRUMENTS_MODULE)
if (BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif (BUILD_INSPECTOR_MODULE)

if (BUILD_AUTOBOT_MODULE)
    add_subdirectory(autobot)
endif()

# Stubs
add_subdirectory(stubs)

if (BUILD_UNIT_TESTS)
#    add_subdirectory(notation/tests) no tests at moment
    add_subdirectory(userscores/tests)

    add_subdirectory(libmscore/tests)
    add_subdirectory(importexport/bb/tests)
    add_subdirectory(importexport/braille/tests)
    add_subdirectory(importexport/bww/tests)
    add_subdirectory(importexport/capella/tests)
    add_subdirectory(importexport/guitarpro/tests)
    add_subdirectory(importexport/midiimport/tests)
    add_subdirectory(importexport/musicxml/tests)
endif(BUILD_UNIT_TESTS)

if (OS_IS_WASM)
    add_subdirectory(wasmtest)
endif()
