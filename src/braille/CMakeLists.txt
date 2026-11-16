muse_create_module(braille)

add_subdirectory(tables)

target_sources(braille PRIVATE
    braillemodule.cpp
    braillemodule.h
    ibrailleconfiguration.h
    ibrailleconverter.h
    inotationbraille.h
    internal/braillecode.cpp
    internal/braillecode.h
    internal/brailleconfiguration.cpp
    internal/brailleconfiguration.h
    internal/brailleconverter.cpp
    internal/brailleconverter.h
    internal/brailleinput.cpp
    internal/brailleinput.h
    internal/brailleinputparser.cpp
    internal/brailleinputparser.h
    internal/braillewriter.cpp
    internal/braillewriter.h
    internal/braille.cpp
    internal/braille.h
    internal/louis.cpp
    internal/louis.h
    internal/notationbraille.cpp
    internal/notationbraille.h
)

target_link_libraries(braille PRIVATE
    engraving
    liblouis
)

# Workaround for an MSVC 14.52 (VS 2026 18.12) ARM64 bug: the loop vectorizer crashes
# with an internal compiler error (C1001) at /O2 on _lou_isValidMode() in liblouis utils.c.
# Reported to Microsoft: https://developercommunity.visualstudio.com/t/MSVC-ARM64-O2-crashes-with-C1001-vector/11183091
# Remove this once the fixed MSVC version is used on the windows-11-arm runners.
if (MSVC AND ARCH_IS_AARCH64)
    get_target_property(_liblouis_sources liblouis SOURCES)
    list(FILTER _liblouis_sources INCLUDE REGEX "/utils\\.c$")
    set_source_files_properties(${_liblouis_sources} TARGET_DIRECTORY liblouis PROPERTIES COMPILE_OPTIONS /Od)
endif()

get_property(_liblouis_tables GLOBAL PROPERTY liblouis_TABLES)
install(FILES ${_liblouis_tables} DESTINATION ${MUSE_APP_INSTALL_RESOURCES_LOCATION}/tables)

if (MUE_BUILD_BRAILLE_TESTS)
    add_subdirectory(tests)
endif()

if (MUE_BUILD_BRAILLE_QML)
    add_subdirectory(qml/MuseScore/Braille)
endif()
