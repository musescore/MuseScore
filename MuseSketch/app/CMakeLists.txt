qt_add_executable(musesketch MACOSX_BUNDLE
    main.cpp
)

qt_add_qml_module(musesketch
    URI "MuseSketch"
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(musesketch
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
)

install(TARGETS musesketch
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
