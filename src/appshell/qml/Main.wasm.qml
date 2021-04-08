import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.15
import MuseScore.NotationScene 1.0
import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import "DevTools/Audio"

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


    RowLayout {
        anchors.fill: parent


        NotationView {
            id: notationView
            Layout.fillWidth: true
            Layout.fillHeight: true

            Component.onCompleted: {
            }
        }

        AudioEngineTests {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }

}
