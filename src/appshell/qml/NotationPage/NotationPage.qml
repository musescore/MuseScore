import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.AppShell 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.Instruments 1.0

DockPage {
    id: notationPage
    objectName: "Notation"

    property var color: ui.theme.backgroundPrimaryColor
    property var borderColor: ui.theme.strokeColor

    property bool isNotationToolBarVisible: false
    property bool isPlaybackToolBarVisible: false
    property bool isUndoRedoToolBarVisible: false
    property bool isNotationNavigatorVisible: false

    property NotationPageModel pageModel: NotationPageModel {}

    property KeyNavigationSection noteInputKeyNavSec: KeyNavigationSection {
        id: keynavSec
        name: "NoteInputSection"
        order: 2
    }

    function updatePageState() {
        var states = [
                    {"Palette": palettePanel.visible},
                    {"Instruments": instrumentsPanel.visible},
                    {"Inspector": inspectorPanel.visible},
                    {"NoteInputBar": notationNoteInputBar.visible},
                    {"NotationToolBar": isNotationToolBarVisible},
                    {"PlaybackToolBar": isPlaybackToolBarVisible},
                    {"UndoRedoToolBar": isUndoRedoToolBarVisible}
                ]

        pageModel.setPanelsState(states)
    }

    Component.onCompleted: {
        updatePageState()

        palettePanel.visible = Qt.binding(function() { return pageModel.isPalettePanelVisible })
        instrumentsPanel.visible = Qt.binding(function() { return pageModel.isInstrumentsPanelVisible })
        inspectorPanel.visible = Qt.binding(function() { return pageModel.isInspectorPanelVisible })
        notationNoteInputBar.visible = Qt.binding(function() { return pageModel.isNoteInputBarVisible })

        pageModel.init()
    }

    toolbar: DockToolBar {
        id: notationNoteInputBar
        objectName: "notationNoteInputBar"

        minimumWidth: orientation == Qt.Horizontal ? 900 : 96
        minimumHeight: orientation == Qt.Horizontal ? 48 : 0

        color: notationPage.color

        title: qsTrc("appshell", "Note Input")

        content: NoteInputBar {
            color: notationNoteInputBar.color
            orientation: notationNoteInputBar.orientation
            keynav.section: noteInputKeyNavSec
            keynav.order: 1
        }
    }

    readonly property int defaultPanelWidth: 272
    readonly property int minimumPanelWidth: 200

    panels: [
        DockPanel {
            id: palettePanel
            objectName: "palettePanel"

            title: qsTrc("appshell", "Palette")

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            color: notationPage.color
            borderColor: notationPage.borderColor

            floatable: true
            closable: true

            onClosed: {
                notationPage.pageModel.isPalettePanelVisible = false
            }

            PalettesWidget {}
        },

        DockPanel {
            id: instrumentsPanel
            objectName: "instrumentsPanel"

            title: qsTrc("appshell", "Instruments")

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            color: notationPage.color
            borderColor: notationPage.borderColor

            tabifyObjectName: "palettePanel"

            floatable: true
            closable: true

            onClosed: {
                notationPage.pageModel.isInstrumentsPanelVisible = false
            }

            InstrumentsPanel {
                anchors.fill: parent
            }
        },

        DockPanel {
            id: inspectorPanel
            objectName: "inspectorPanel"

            title: qsTrc("appshell", "Properties")

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            color: notationPage.color
            borderColor: notationPage.borderColor

            tabifyObjectName: "instrumentsPanel"

            floatable: true
            closable: true

            onClosed: {
                notationPage.pageModel.isInspectorPanelVisible = false
            }

            InspectorForm {
                anchors.fill: parent
            }
        }
    ]

    central: DockCentral {
        id: notationCentral
        objectName: "notationCentral"

        NotationView {
            id: notationView

            isNavigatorVisible: notationPage.pageModel.isNotationNavigatorVisible

            onTextEdittingStarted: {
                notationCentral.forceActiveFocus()
            }
        }
    }

    statusbar: DockStatusBar {
        id: notationStatusBar
        objectName: "notationStatusBar"

        width: notationPage.width
        color: notationPage.color

        visible: notationPage.pageModel.isStatusBarVisible

        NotationStatusBar {
            anchors.fill: parent
            color: notationStatusBar.color
        }
    }
}
