muse_create_qml_module(muse_workspace_qml ALIAS muse::workspace_qml FOR muse_workspace)

qt_add_qml_module(muse_workspace_qml
    URI Muse.Workspace
    VERSION 1.0
    SOURCES
        newworkspacemodel.cpp
        newworkspacemodel.h
        workspacelistmodel.cpp
        workspacelistmodel.h
        workspacesmenumodel.cpp
        workspacesmenumodel.h
    QML_FILES
        NewWorkspaceDialog.qml
        WorkspacesDialog.qml
        internal/WorkspacesTopPanel.qml
        internal/WorkspacesView.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_workspace_qml)
