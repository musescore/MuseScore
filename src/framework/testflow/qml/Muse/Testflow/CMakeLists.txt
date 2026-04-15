muse_create_qml_module(muse_testflow_qml ALIAS muse::testflow_qml FOR muse_testflow)

qt_add_qml_module(muse_testflow_qml
    URI Muse.Testflow
    VERSION 1.0
    SOURCES
        testflowfilesmodel.cpp
        testflowfilesmodel.h
        testflowscriptsmodel.cpp
        testflowscriptsmodel.h
        testcaserunmodel.cpp
        testcaserunmodel.h
    QML_FILES
        TestflowSelectFileDialog.qml
        ScriptsDialog.qml
        ScriptsPanel.qml
        TestCaseRunPanel.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_testflow_qml)
