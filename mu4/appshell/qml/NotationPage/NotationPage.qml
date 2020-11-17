import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.Instruments 1.0

DockPage {
    id: notationPage
    objectName: "Notation"

    property var color: ui.theme.backgroundPrimaryColor

    toolbar: DockToolBar {
        id: notationToolBar
        objectName: "notationToolBar"

        minimumWidth: orientation == Qt.Horizontal ? 600 : 96
        minimumHeight: orientation == Qt.Horizontal ? 48 : 0

        color: notationPage.color

        content: NotationToolBar {
            id: _toolbar
            anchors.fill: parent
            color: notationToolBar.color

            orientation: notationToolBar.orientation
        }
    }

    panels: [
        DockPanel {
            id: palettePanel
            objectName: "palettePanel"

            title: qsTrc("appshell", "Palette")
            width: 294
            color: notationPage.color

            PalettesWidget {}
        },

        DockPanel {
            id: instrumentsPanel
            objectName: "instrumentsPanel"

            title: qsTrc("appshell", "Instruments")
            width: 294
            color: notationPage.color

            tabifyObjectName: "palettePanel"

            InstrumentsPanel {
                anchors.fill: parent
            }
        },

        DockPanel {
            id: inspectorPanel
            objectName: "inspectorPanel"

            title: qsTrc("appshell", "Inspector")
            width: 294
            color: notationPage.color

            tabifyObjectName: "instrumentsPanel"

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
        }
    }

    statusbar: DockStatusBar {
        id: notationStatusBar
        objectName: "notationStatusBar"

        width: notationPage.width
        color: notationPage.color

        NotationStatusBar {
            anchors.fill: parent
            color: notationStatusBar.color
        }
    }
}
