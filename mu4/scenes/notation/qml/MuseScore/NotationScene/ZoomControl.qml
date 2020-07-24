import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    id: root

    spacing: 8

    ZoomControlModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    StyledIconLabel {
        width: 40

        iconCode: IconCode.ZOOM_OUT

        MouseArea {
            anchors.fill: parent
            onClicked: model.zoomOut()
        }
    }

    StyledTextLabel {
        width: 60
        text: model.currentZoom + " %"
    }

    StyledIconLabel {
        width: 40

        iconCode: IconCode.ZOOM_IN

        MouseArea {
            anchors.fill: parent
            onClicked: model.zoomIn()
        }
    }
}
