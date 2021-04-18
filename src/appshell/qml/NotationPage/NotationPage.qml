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

    property KeyNavigationSection keynavNoteInputSec: KeyNavigationSection {
        name: "NoteInputSection"
        order: 2
    }

    property KeyNavigationSection keynavLeftPanelSec: KeyNavigationSection {
        name: "LeftPanel"
        order: 3
    }

    property KeyNavigationSection keynavRightPanelSec: KeyNavigationSection {
        name: "RightPanel"
        order: 4
    }

    property KeyNavigationSubSection keynavLeftPanelTabsSubSec: KeyNavigationSubSection {
        name: "LeftPanelTabs"
        section: keynavLeftPanelSec
        order: 1
    }

    property KeyNavigationSubSection keynavRightPanelTabsSubSec: KeyNavigationSubSection {
        name: "RightPanelTabs"
        section: keynavRightPanelSec
        order: 1
    }

    function keynavPanelTabSubSec(area) {
        if (area === Qt.LeftDockWidgetArea) {
            return keynavLeftPanelTabsSubSec
        }
        return keynavRightPanelTabsSubSec
    }

    function keynavPanelSec(area) {
        if (area === Qt.LeftDockWidgetArea) {
            return keynavLeftPanelSec
        }
        return keynavRightPanelSec
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
            keynav.section: keynavNoteInputSec
            keynav.order: 1
        }
    }

    readonly property int defaultPanelWidth: 272
    readonly property int minimumPanelWidth: 200

    panels: [
        DockPanel {
            id: palettePanel
            objectName: "palettePanel"

            property string _title: qsTrc("appshell", "Palette")
            title: palettePanel.keynavTab.active ? ("[" + _title + "]") : _title //! NOTE just for test

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            color: notationPage.color
            borderColor: notationPage.borderColor

            floatable: true
            closable: true

            onClosed: {
                notationPage.pageModel.isPalettePanelVisible = false
            }

            property KeyNavigationControl keynavTab: KeyNavigationControl {
                name: "PaletteTab"
                order: 1 //! TODO Needs order from DockPanel
                subsection: notationPage.keynavPanelTabSubSec(palettePanel.area)
                onActiveChanged: {
                    if (active) {
                        palettePanel.forceActiveFocus()
                    }
                }
            }

            PalettesWidget {
                anchors.fill: parent
                keynavSection: notationPage.keynavPanelSec(palettePanel.area)
            }
        },

        DockPanel {
            id: instrumentsPanel
            objectName: "instrumentsPanel"

            property string _title: qsTrc("appshell", "Instruments")
            title: instrumentsPanel.keynavTab.active ? ("[" + _title + "]") : _title //! NOTE just for test

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

            property KeyNavigationControl keynavTab: KeyNavigationControl {
                name: "InstrumentsTab"
                order: 2 //! TODO Needs order from DockPanel
                subsection: notationPage.keynavPanelTabSubSec(instrumentsPanel.area)
                onActiveChanged: {
                    if (active) {
                        instrumentsPanel.forceActiveFocus()
                    }
                }
            }

            InstrumentsPanel {
                anchors.fill: parent
                keynavSection: notationPage.keynavPanelSec(instrumentsPanel.area)
                visible: instrumentsPanel.isShown
            }
        },

        DockPanel {
            id: inspectorPanel
            objectName: "inspectorPanel"

            property string _title: qsTrc("appshell", "Properties")
            title: inspectorPanel.keynavTab.active ? ("[" + _title + "]") : _title //! NOTE just for test

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

            property KeyNavigationControl keynavTab: KeyNavigationControl {
                name: "InspectorTab"
                order: 3 //! TODO Needs order from DockPanel
                subsection: notationPage.keynavPanelTabSubSec(inspectorPanel.area)
                onActiveChanged: {
                    if (active) {
                        inspectorPanel.forceActiveFocus()
                    }
                }
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
