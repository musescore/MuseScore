muse_create_qml_module(muse_workspace_qml ALIAS muse::workspace_qml FOR muse_workspace STUB)

qt_add_qml_module(muse_workspace_qml
    URI Muse.Workspace
    VERSION 1.0
    QML_FILES
        WorkspacesDialog.qml
        NewWorkspaceDialog.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_workspace_qml)
