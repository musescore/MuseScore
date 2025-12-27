muse_create_qml_module(braille_qml FOR braille)

qt_add_qml_module(braille_qml
    URI MuseScore.Braille
    VERSION 1.0
    QML_FILES
        BrailleView.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(braille_qml)
