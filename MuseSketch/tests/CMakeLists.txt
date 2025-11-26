# Tests are enabled in the top-level CMakeLists.txt

# MotifQuantizer tests
add_executable(test_motif_quantizer
    tst_motif_quantizer.cpp
)

target_link_libraries(test_motif_quantizer PRIVATE
    Qt6::Core
    Qt6::Test
    musesketch_core
)

add_test(NAME MotifQuantizerTest COMMAND test_motif_quantizer)

# RhythmGrid tests
add_executable(test_rhythm_grid
    tst_rhythm_grid.cpp
)

target_link_libraries(test_rhythm_grid PRIVATE
    Qt6::Core
    Qt6::Test
    musesketch_core
)

add_test(NAME RhythmGridTest COMMAND test_rhythm_grid)

# Motif CRUD tests (PR-06)
add_executable(test_motif_crud
    tst_motif_crud.cpp
)

target_link_libraries(test_motif_crud PRIVATE
    Qt6::Core
    Qt6::Test
    musesketch_core
)

add_test(NAME MotifCrudTest COMMAND test_motif_crud)

# Section tests (PR-07)
add_executable(test_section
    tst_section.cpp
)

target_link_libraries(test_section PRIVATE
    Qt6::Core
    Qt6::Test
    musesketch_core
)

add_test(NAME SectionTest COMMAND test_section)

