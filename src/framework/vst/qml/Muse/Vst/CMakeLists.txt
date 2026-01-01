muse_create_qml_module(muse_vst_qml ALIAS muse::vst_qml FOR muse_vst)

qt_add_qml_module(muse_vst_qml
    URI Muse.Vst
    VERSION 1.0
    SOURCES
        vstview.cpp
        vstview.h
    QML_FILES
        VstEditor.qml
        VstEditorDialog.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
)

fixup_qml_module_dependencies(muse_vst_qml)

if (OS_IS_WIN)
    target_sources(muse_vst_qml PRIVATE
        platform/windows/flickeringworkaround.cpp
        platform/windows/flickeringworkaround.h
    )
endif()

if (OS_IS_LIN OR OS_IS_FBSD)
    target_sources(muse_vst_qml PRIVATE
        platform/linux/runloop.cpp
        platform/linux/runloop.h
    )
endif()

target_link_libraries(muse_vst_qml PRIVATE vst3sdk)
