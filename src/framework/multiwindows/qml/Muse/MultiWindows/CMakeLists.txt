muse_create_qml_module(muse_multiwindows_qml ALIAS muse::multiwindows_qml FOR muse_multiwindows)

qt_add_qml_module(muse_multiwindows_qml
    URI Muse.MultiWindows
    VERSION 1.0
    SOURCES
        multiinstancesdevmodel.cpp
        multiinstancesdevmodel.h
    QML_FILES
        MultiInstancesDevDialog.qml
        MultiInstancesDevPanel.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_multiwindows_qml)

# Compile Qt generated caches separately to avoid generic Type collisions.
set_target_properties(muse_multiwindows_qml PROPERTIES UNITY_BUILD OFF)
