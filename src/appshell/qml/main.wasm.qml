import QtQuick 2.0
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0
import MuseScore.Playback 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480

    visible: true
    title: qsTr("Muse Score")

    header: ToolBar {
        contentHeight: 40

        PlaybackToolBar {
            id: playbackToolbar
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            color: ui.theme.backgroundPrimaryColor
        }

        Label {
            text: "position: "
            anchors.centerIn: parent
        }
    }


    NotationView {
        id: notationView
        anchors.fill: parent

        Component.onCompleted: {
        }
    }

}
