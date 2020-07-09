import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0

DockPage {
    id: notationPage
    objectName: "Notation"

    property var color: ui.theme.backgroundColor

    toolbar: DockToolBar {
        id: notationToolBar
        objectName: "notationToolBar"

        height: 40
        width: 400
        color: notationPage.color

        NotationToolBar {
            color: notationToolBar.color
        }
    }

    panels: [
        DockPanel {

            id: palettePanel
            objectName: "palettePanel"

            title: "Palette"
            width: 200
            color: notationPage.color

            PalettesWidget {

            }
        },

        DockPanel {

            id: inspectorPanel
            objectName: "inspectorPanel"

            title: "Inspector"
            width: 200
            color: notationPage.color

            //area: Qt.RightDockWidgetArea
            tabifyObjectName: "palettePanel"

            Rectangle {

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: "Inspector"
                }
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

        Rectangle {
            color: notationStatusBar.color

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                text: "Notation status bar"
            }
        }
    }
}
