import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    id: root

    spacing: 4

    ZoomControlModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    FlatButton {
        icon: IconCode.ZOOM_IN

        normalStateColor: "transparent"

        onClicked: {
            model.zoomIn()
        }
    }

    FlatButton {
        icon: IconCode.ZOOM_OUT

        normalStateColor: "transparent"

        onClicked: {
            model.zoomOut()
        }
    }

    StyledTextLabel {
        height: parent.height

        width: 60

        text: model.currentZoom + " %"
    }
}
