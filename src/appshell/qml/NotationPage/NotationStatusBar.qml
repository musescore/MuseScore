import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Workspace 1.0

Rectangle {
    id: root

    NotationAccessibilityInfo {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 20

        spacing: 12

        WorkspacesControl {
            anchors.verticalCenter: parent.verticalCenter
        }

        ConcertPitchControl {
            anchors.verticalCenter: parent.verticalCenter
        }

        ViewModeControl {
            anchors.verticalCenter: parent.verticalCenter
        }

        ZoomControl {
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
