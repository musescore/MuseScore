
add_subdirectory(framework)

if (NOT BUILD_USERSCORES_MODULE)
    add_subdirectory(userscores)
endif (NOT BUILD_USERSCORES_MODULE)

if (NOT BUILD_EXTENSIONS_MODULE)
    add_subdirectory(extensions)
endif (NOT BUILD_EXTENSIONS_MODULE)

if (NOT BUILD_LANGUAGES_MODULE)
    add_subdirectory(languages)
endif (NOT BUILD_LANGUAGES_MODULE)
