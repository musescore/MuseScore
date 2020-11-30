import QtQuick 2.7

import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0

DockPage {
    id: sequencerPage

    objectName: "Sequencer"

    property var color: ui.theme.backgroundPrimaryColor

    toolbar: DockToolBar {

        id: seqToolBar
        objectName: "seqToolBar"

        minimumWidth: 400
        minimumHeight: 40
        color: sequencerPage.color

        Rectangle {
            color: sequencerPage.color

            StyledTextLabel {
                anchors.fill: parent
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

                StyledTextLabel {
                    anchors.fill: parent
                    text: "Mixer"
                }
            }
        }
    ]

    central: DockCentral {

        id: seqCentral
        objectName: "seqCentral"

        Rectangle {
            StyledTextLabel {
                anchors.fill: parent
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

            StyledTextLabel {
                anchors.fill: parent
                text: "Notation status bar"
            }
        }
    }
}
