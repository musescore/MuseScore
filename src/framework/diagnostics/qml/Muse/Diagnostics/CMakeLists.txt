muse_create_qml_module(muse_diagnostics_qml ALIAS muse::diagnostics_qml FOR muse_diagnostics)

qt_add_qml_module(muse_diagnostics_qml
    URI Muse.Diagnostics
    VERSION 1.0
    SOURCES
        actionsviewmodel.cpp
        actionsviewmodel.h
        devtools/crashhandlerdevtoolsmodel.cpp
        devtools/crashhandlerdevtoolsmodel.h
        diagnosticaccessiblemodel.cpp
        diagnosticaccessiblemodel.h
        diagnosticspathsmodel.cpp
        diagnosticspathsmodel.h
        keynav/abstractkeynavdevitem.cpp
        keynav/abstractkeynavdevitem.h
        keynav/diagnosticnavigationmodel.cpp
        keynav/diagnosticnavigationmodel.h
        keynav/keynavdevcontrol.cpp
        keynav/keynavdevcontrol.h
        keynav/keynavdevsection.cpp
        keynav/keynavdevsection.h
        keynav/keynavdevsubsection.cpp
        keynav/keynavdevsubsection.h
        system/graphicsinfomodel.cpp
        system/graphicsinfomodel.h
        system/profilerviewmodel.cpp
        system/profilerviewmodel.h
    QML_FILES
        DiagnosticAccessibleDialog.qml
        DiagnosticAccessiblePanel.qml
        DiagnosticActionsDialog.qml
        DiagnosticActionsPanel.qml
        DiagnosticGraphicsInfoDialog.qml
        DiagnosticGraphicsInfoPanel.qml
        DiagnosticNavigationDialog.qml
        DiagnosticNavigationPanel.qml
        DiagnosticPathsDialog.qml
        DiagnosticPathsPanel.qml
        DiagnosticProfilerDialog.qml
        DiagnosticProfilerPanel.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_diagnostics_qml)

# Necessary for the auto-generated sources
target_include_directories(muse_diagnostics_qml PRIVATE devtools keynav system)