import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.NotationScene 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "internal"

Rectangle {
    id: root

    width: contentRow.width
    height: contentRow.height

    property var items: [
        {
            title: qsTrc("notation", "Parts"),
            icon: IconCode.NEW_FILE,
            uri: "musescore://notation/parts"
        },
        {
            title: qsTrc("notation", "Mixer"),
            icon: IconCode.MIXER,
            uri: "musescore://notation/mixer"
        }
    ]

    Row {
        id: contentRow

        anchors.verticalCenter: parent.verticalCenter

        spacing: 12

        Repeater {
            anchors.fill: parent
            model: items

            FlatButton {
                text: modelData["title"]
                icon: modelData["icon"]

                normalStateColor: "transparent"
                orientation: Qt.Horizontal

                onClicked: {
                    api.launcher.open(modelData["uri"])
                }
            }
        }
    }
}
