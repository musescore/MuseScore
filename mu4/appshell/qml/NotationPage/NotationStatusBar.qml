import QtQuick 2.7
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    NotationAccessibilityInfo {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
    }

    ViewModeControl {
        id: viewModeControl
        anchors.right: zoomControl.left
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
    }

    ZoomControl {
        id: zoomControl
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
    }
}
