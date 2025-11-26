# For PR-02, we're creating a minimal placeholder implementation.
# Full MuseScore core integration will be done in a future PR once we have
# a better understanding of the minimal dependencies needed.

add_library(musesketch_core STATIC
    include/ScoreEngine.h
    include/Sketch.h
    include/Motif.h
    include/Section.h
    include/RhythmGrid.h
    include/MotifQuantizer.h
    include/SketchRepository.h
    include/SketchManager.h
    include/MotifEditorController.h
    include/AudioEngine.h
    include/MotifPlayer.h
    include/PartwritingEngine.h
    src/ScoreEngine.cpp
    src/Sketch.cpp
    src/Motif.cpp
    src/Section.cpp
    src/RhythmGrid.cpp
    src/MotifQuantizer.cpp
    src/SketchRepository.cpp
    src/SketchManager.cpp
    src/MotifEditorController.cpp
    src/AudioEngine.cpp
    src/MotifPlayer.cpp
    src/PartwritingEngine.cpp
)

target_include_directories(musesketch_core PUBLIC
    include
)

target_link_libraries(musesketch_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
)
