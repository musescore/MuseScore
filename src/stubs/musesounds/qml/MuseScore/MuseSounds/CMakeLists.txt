muse_create_qml_module(musesounds_qml STUB FOR musesounds)

qt_add_qml_module(musesounds_qml
    URI MuseScore.MuseSounds
    VERSION 1.0
    QML_FILES
        MuseSoundsPage.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(musesounds_qml)

# Compile Qt generated caches separately to avoid generic Type collisions.
set_target_properties(musesounds_qml PROPERTIES UNITY_BUILD OFF)
