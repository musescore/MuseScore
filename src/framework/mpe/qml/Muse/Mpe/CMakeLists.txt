muse_create_qml_module(muse_mpe_qml ALIAS muse::mpe_qml FOR muse_mpe)

qt_add_qml_module(muse_mpe_qml
    URI Muse.Mpe
    VERSION 1.0
    SOURCES
        articulationpatternsegmentitem.cpp
        articulationpatternsegmentitem.h
        articulationpatternitem.cpp
        articulationpatternitem.h
        articulationsprofileeditormodel.cpp
        articulationsprofileeditormodel.h
    QML_FILES
        ArrangementPatternControl.qml
        ArticulationPatternControlsTab.qml
        ArticulationPatternPlot.qml
        ArticulationPatternSegmentsList.qml
        ArticulationPatternsList.qml
        ArticulationPatternsTab.qml
        ExpressionPatternControl.qml
        PitchPatternControl.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_mpe_qml)
