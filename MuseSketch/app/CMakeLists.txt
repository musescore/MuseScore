set(APP_NAME musesketch)

# Platform-specific settings
if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android" CACHE INTERNAL "")
endif()

qt_add_executable(${APP_NAME}
    main.cpp
)

# Set bundle properties for iOS/macOS
set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.musescore.musesketch"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist"
)

# iOS-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set_target_properties(${APP_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.musescore.musesketch"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone and iPad
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
        # Qt generates LaunchScreen automatically, so we don't add our own
    )
endif()

# Android-specific settings
if(ANDROID)
    set_target_properties(${APP_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_VERSION_CODE 1
    )
endif()

qt_add_qml_module(${APP_NAME}
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

target_link_libraries(${APP_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Multimedia
    musesketch_core
)

install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
