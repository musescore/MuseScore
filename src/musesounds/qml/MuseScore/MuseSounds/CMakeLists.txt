muse_create_qml_module(musesounds_qml FOR musesounds)

qt_add_qml_module(musesounds_qml
    URI MuseScore.MuseSounds
    VERSION 1.0
    SOURCES
        musesoundslistmodel.cpp
        musesoundslistmodel.h
    QML_FILES
        MuseSoundsPage.qml
        internal/SoundCatalogueListView.qml
        internal/SoundLibraryItem.qml
        MuseSoundsReleaseInfoDialog.qml
        internal/MuseSoundsReleaseInfoBottomPanel.qml
    RESOURCES
        resources/muse_sounds_promo.png
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(musesounds_qml)
