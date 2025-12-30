muse_create_qml_module(muse_dockwindow_qml ALIAS muse::dockwindow_qml FOR muse_dockwindow)

qt_add_qml_module(muse_dockwindow_qml
    URI Muse.Dock
    VERSION 1.0
    SOURCES
        dockcentralview.cpp
        dockcentralview.h
        dockframemodel.cpp
        dockframemodel.h
        dockingholderview.cpp
        dockingholderview.h
        dockpageview.cpp
        dockpageview.h
        dockpanelview.cpp
        dockpanelview.h
        dockstatusbar.cpp
        dockstatusbar.h
        docktabsmodel.cpp
        docktabsmodel.h
        docktoolbarview.cpp
        docktoolbarview.h
        dockwindow.cpp
        dockwindow.h
        foreign.cpp
        foreign.h
    QML_FILES
        DockFloatingWindow.qml
        DockFrame.qml
        DockingHolder.qml
        DockPage.qml
        DockPanel.qml
        DockPanelTab.qml
        DockSeparator.qml
        DockTabBar.qml
        DockTitleBar.qml
        DockTitleBarMouseArea.qml
        DockToolBar.qml
        DockWidget.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_dockwindow_qml)

target_link_libraries(muse_dockwindow_qml PRIVATE kddockwidgets)
