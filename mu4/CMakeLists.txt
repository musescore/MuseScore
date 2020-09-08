

add_subdirectory(appshell)
add_subdirectory(context)
add_subdirectory(cloud)

# Domain
add_subdirectory(notation)
add_subdirectory(importexport)

# Home
add_subdirectory(userscores)
add_subdirectory(extensions)
add_subdirectory(languages)
add_subdirectory(plugins)

# Scenes common
add_subdirectory(commonscene)
add_subdirectory(playback)

# Notation
add_subdirectory(palette)
add_subdirectory(inspector)
add_subdirectory(instruments)

if (BUILD_UNIT_TESTS)
    add_subdirectory(notation/tests)
    add_subdirectory(userscores/tests)
endif(BUILD_UNIT_TESTS)
