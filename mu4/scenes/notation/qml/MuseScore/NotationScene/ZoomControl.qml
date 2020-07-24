import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Row {
    id: root

    property string currentZoom: ""

    signal zoomInRequested()
    signal zoomOutRequested()

    spacing: 8

    StyledTextLabel {
        width: 40

        text: "-"
        font.bold: true

        MouseArea {
            anchors.fill: parent
            onClicked: root.zoomOutRequested()
        }
    }

    StyledTextLabel {
        width: 60
        text: root.currentZoom + " %"
    }

    StyledTextLabel {
        width: 40

        text: "+"
        font.bold: true

        MouseArea {
            anchors.fill: parent
            onClicked: root.zoomInRequested()
        }
    }
}
