muse_create_qml_module(muse_midiremote_qml ALIAS muse::midiremote_qml FOR muse_midiremote)

qt_add_qml_module(muse_midiremote_qml
    URI Muse.MidiRemote
    VERSION 1.0
    SOURCES
        editmidimappingmodel.cpp
        editmidimappingmodel.h
        mididevicemappingmodel.cpp
        mididevicemappingmodel.h
    QML_FILES
        EditMidiMappingDialog.qml
        internal/MidiMappingBottomPanel.qml
        internal/MidiMappingTopPanel.qml
        MidiDeviceMappingPage.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_midiremote_qml)