muse_create_module(braille)

add_subdirectory(tables)
add_subdirectory(thirdparty/liblouis)

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

if (MUE_BUILD_BRAILLE_TESTS)
    add_subdirectory(tests)
endif()

if (MUE_BUILD_BRAILLE_QML)
    add_subdirectory(qml/MuseScore/Braille)
endif()
