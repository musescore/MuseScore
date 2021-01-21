
add_subdirectory(framework)


if (NOT BUILD_LANGUAGES_MODULE)
    add_subdirectory(languages)
endif (NOT BUILD_LANGUAGES_MODULE)
