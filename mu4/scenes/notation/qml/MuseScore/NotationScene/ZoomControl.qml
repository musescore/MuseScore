import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    id: root

    property string currentZoom: ""

    signal zoomInRequested()
    signal zoomOutRequested()

    spacing: 8

    StyledIconLabel {
        width: 40

        iconCode: IconCode.ZOOM_OUT

        MouseArea {
            anchors.fill: parent
            onClicked: root.zoomOutRequested()
        }
    }

    StyledTextLabel {
        width: 60
        text: root.currentZoom + " %"
    }

    StyledIconLabel {
        width: 40

        iconCode: IconCode.ZOOM_IN

        MouseArea {
            anchors.fill: parent
            onClicked: root.zoomInRequested()
        }
    }
}
