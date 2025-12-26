muse_create_qml_module(muse_multiinstances_qml ALIAS muse::multiinstances_qml FOR muse_multiinstances)

qt_add_qml_module(muse_multiinstances_qml
    URI Muse.MultiInstances
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

fixup_qml_module_dependencies(muse_multiinstances_qml)
