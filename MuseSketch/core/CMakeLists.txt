# For PR-02, we're creating a minimal placeholder implementation.
# Full MuseScore core integration will be done in a future PR once we have
# a better understanding of the minimal dependencies needed.

add_library(musesketch_core STATIC
    src/ScoreEngine.cpp
)

target_include_directories(musesketch_core PUBLIC
    include
)

target_link_libraries(musesketch_core PRIVATE
    Qt6::Core
    Qt6::Gui
)
