import QtQuick 2.9
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0

DockPage {
    id: sequencerPage

    objectName: "sequencer"

    property var color: ui.theme.window

    toolbar: DockToolBar {

        id: seqToolBar
        objectName: "seqToolBar"

        height: 40
        width: 400
        color: sequencerPage.color

        Rectangle {
            color: sequencerPage.color

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                text: "Sequencer toolbar"
            }
        }
    }

    panels: [
        DockPanel {

            id: mixerPanel
            objectName: "mixerPanel"

            title: "Mixer"
            width: 200

            Rectangle {

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: "Mixer"
                }
            }
        }
    ]

    central: DockCentral {

        id: seqCentral
        objectName: "seqCentral"

        Rectangle {

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Sequencer"
            }
        }
    }

    statusbar: DockStatusBar {

        id: notationStatusBar
        objectName: "seqStatusBar"

        width: 400
        color: sequencerPage.color

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
