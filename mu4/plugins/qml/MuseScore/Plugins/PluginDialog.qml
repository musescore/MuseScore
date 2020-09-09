import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.Plugins 1.0

QmlDialog {
    id: root

    property string name: ""
    property string url: ""

    width: 400
    height: 400
    title: name

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        Loader {
            id: loader
            anchors.centerIn: parent
            source: root.url
        }
    }
}
