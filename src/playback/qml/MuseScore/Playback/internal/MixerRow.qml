import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0
import MuseScore.Playback 1.0

Row {
    id: root

    property real spacingAbove: 4
    property real spacingBelow: 4

    property alias header: headerLoader.sourceComponent
    property alias body: bodyLoader.sourceComponent
    property var flickableArea




    width: implicitWidth
    height: root.spacingAbove + (bodyLoader.status === Loader.Ready ? bodyLoader.item.contentHeight : 0) + root.spacingBelow
    // height: 150

    spacing: 1 // for separator (will be rendered in MixerPanel.qml)

    // visible: (headerLoader.status === Loader.Ready) && (bodyLoader.status === Loader.ready)
    visible: true

    Loader {
        id: headerLoader
        z: 2
    }

    Loader {
        id: bodyLoader
        z: 1
    }

    Connections {
        target: root.flickableArea
        function onContentXChanged() {
            if (root.flickableArea && headerLoader.item) {
                console.log(root.flickableArea.contentX)
                headerLoader.item.x = Math.max(0, root.flickableArea.contentX)
            }
        }
    }


}