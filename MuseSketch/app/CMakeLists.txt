qt_add_executable(musesketch MACOSX_BUNDLE
    main.cpp
)

qt_add_qml_module(musesketch
    URI "MuseSketch"
    VERSION 1.0
    QML_FILES 
        Main.qml
        SketchView.qml
        MotifShapeView.qml
        MotifEditView.qml
        SectionEditorView.qml
        ExportView.qml
        components/ScaleDegreePad.qml
        components/RhythmButton.qml
        components/MotifDisplay.qml
        components/ContourCanvas.qml
)

target_link_libraries(musesketch PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    musesketch_core
)

install(TARGETS musesketch
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
