
# Main
add_subdirectory(main)

# Framework
add_subdirectory(framework)

# Domain model
add_subdirectory(libmscore)

# AppShell
add_subdirectory(appshell)

# Modules
add_subdirectory(audiofile)
add_subdirectory(context)
add_subdirectory(notation)
add_subdirectory(userscores)
add_subdirectory(converter)
add_subdirectory(workspace)

if (NOT OS_IS_WASM)
    add_subdirectory(importexport)
    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/beatroot beatroot) # for midiimport
    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rtf2html rtf2html) # for capella

    add_subdirectory(cloud)
    add_subdirectory(extensions)
    add_subdirectory(languages)
    add_subdirectory(plugins)
endif()

# Scenes common
add_subdirectory(commonscene)
add_subdirectory(playback)

# Notation
add_subdirectory(palette)
add_subdirectory(inspector)
add_subdirectory(instruments)

# Stubs
add_subdirectory(stubs)

if (BUILD_UNIT_TESTS)
#    add_subdirectory(notation/tests) no tests at moment
    add_subdirectory(userscores/tests)
    add_subdirectory(libmscore/tests)
    add_subdirectory(importexport/tests)
endif(BUILD_UNIT_TESTS)

if (OS_IS_WASM)
    add_subdirectory(wasmtest)
endif()
