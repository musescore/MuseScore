muse_create_qml_module(muse_autobot_qml ALIAS muse::autobot_qml FOR muse_autobot)

qt_add_qml_module(muse_autobot_qml
    URI Muse.Autobot
    VERSION 1.0
    SOURCES
        abfilesmodel.cpp
        abfilesmodel.h
        autobotscriptsmodel.cpp
        autobotscriptsmodel.h
        testcaserunmodel.cpp
        testcaserunmodel.h
    QML_FILES
        AutobotSelectFileDialog.qml
        ScriptsDialog.qml
        ScriptsPanel.qml
        TestCaseRunPanel.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_autobot_qml)
