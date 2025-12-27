muse_create_qml_module(muse_shortcuts_qml ALIAS muse::shortcuts_qml FOR muse_shortcuts)

qt_add_qml_module(muse_shortcuts_qml
    URI Muse.Shortcuts
    VERSION 1.0
    SOURCES
        editmidimappingmodel.cpp
        editmidimappingmodel.h
        editshortcutmodel.cpp
        editshortcutmodel.h
        mididevicemappingmodel.cpp
        mididevicemappingmodel.h
        shortcutsinstancemodel.cpp
        shortcutsinstancemodel.h
        shortcutsmodel.cpp
        shortcutsmodel.h
    QML_FILES
        EditMidiMappingDialog.qml
        EditShortcutDialogContent.qml
        internal/MidiMappingBottomPanel.qml
        internal/MidiMappingTopPanel.qml
        internal/ShortcutsBottomPanel.qml
        internal/ShortcutsList.qml
        internal/ShortcutsTopPanel.qml
        MidiDeviceMappingPage.qml
        Shortcuts.qml
        ShortcutsPage.qml
        StandardEditShortcutDialog.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_shortcuts_qml)

if (OS_IS_MAC)
    find_library(CARBON_LIBRARY Carbon)

    target_sources(muse_shortcuts_qml PRIVATE
        platform/macos/macosshortcutsinstancemodel.mm
        platform/macos/macosshortcutsinstancemodel.h
    )

    set_source_files_properties(
        platform/macos/macosshortcutsinstancemodel.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )

    target_include_directories(muse_shortcuts_qml PRIVATE
        ${CARBON_LIBRARY}
    )

    # Necessary for the auto-generated sources
    target_include_directories(muse_shortcuts_qml PRIVATE platform/macos)
endif(OS_IS_MAC)
