import QtQuick 2.15
import MuseScore.Ui 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundSecondaryColor

    Column {
        anchors.fill: parent

        KeyNavSection {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#fce94f"

            sectionName: "sect 2"
            sectionOrder: 2
            subsecCount: 3
        }

        KeyNavSection {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#e9b96e"

            sectionName: "sect 1"
            sectionOrder: 1
            subsecCount: 1
        }

        KeyNavSection {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#729fcf"

            sectionName: "sect 3"
            sectionOrder: 3
            subsecCount: 2
        }

        KeyNavSection {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#8ae234"

            sectionName: "sect 4"
            sectionOrder: 4
            subsecCount: 4
        }

        KeyNavSection {
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#ef2929"

            sectionName: "sect 5"
            sectionOrder: 5
            subsecCount: 2
        }
    }
}
