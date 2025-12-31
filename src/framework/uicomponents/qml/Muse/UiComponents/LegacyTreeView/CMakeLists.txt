muse_create_qml_module(muse_legacytreeview_qml ALIAS muse::legacytreeview_qml FOR muse_uicomponents)

qt_add_qml_module(muse_legacytreeview_qml
    URI Muse.UiComponents.LegacyTreeView
    VERSION 1.0
    SOURCES
        qquicktreemodeladaptor.cpp
        qquicktreemodeladaptor_p.h
        qquickabstractstyle.cpp
        qquickabstractstyle_p.h
        qquickpadding_p.h
        qquickselectionmode_p.h
    QML_FILES
        BasicTableView.qml
        BasicTableViewStyle.qml
        LegacyTreeView.qml
        LegacyTreeViewItemDelegateLoader.qml
        LegacyTreeViewStyle.qml
        Style.qml
        TableViewColumn.qml
        TableViewItemDelegateLoader.qml
    RESOURCES
        images/header.png
    IMPORTS
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_legacytreeview_qml)