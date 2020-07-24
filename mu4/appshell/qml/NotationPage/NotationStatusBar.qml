import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    NotationStatusBarModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter

        text: model.accessibilityInfo
    }

    ZoomControl {
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter

        currentZoom: model.currentZoom
        onZoomInRequested: model.zoomIn()
        onZoomOutRequested: model.zoomOut()
    }
}
