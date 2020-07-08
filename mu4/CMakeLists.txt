

add_subdirectory(appshell)
add_subdirectory(context)
add_subdirectory(account)

# Domain
add_subdirectory(domain/notation)
add_subdirectory(domain/importexport)

# Home
add_subdirectory(scores)
add_subdirectory(extensions)

# Scenes common
add_subdirectory(scenes/common)

# Notation
add_subdirectory(scenes/notation)
add_subdirectory(scenes/palette)

if (BUILD_UNIT_TESTS)
    add_subdirectory(scenes/notation/tests)
endif(BUILD_UNIT_TESTS)
