import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

Item {
    id: root

    property var title: ""
    property alias thumbnail: loader.thumbnail
    property bool isAdd: false

    signal clicked()

    Column {
        anchors.fill: parent

        spacing: 10

        Loader {
            id: loader

            property var thumbnail: undefined

            height: 125
            width: parent.width

            sourceComponent: isAdd ? addComp : thumbnailComp

            onLoaded: {
                if (!isAdd) {
                    item.setThumbnail(thumbnail)
                }
            }
        }

        Text {
            id: scoreTitle
            anchors.horizontalCenter: parent.horizontalCenter

            text: title
            color: "#FFFFFF"
        }
    }

    Component {
        id: thumbnailComp

        ScoreThumbnail {
            anchors.fill: parent
        }
    }

    Component {
        id: addComp

        Rectangle {
            anchors.fill: parent

            color: "#FFFFFF"

            Image {
                anchors.centerIn: parent

                sourceSize.width: 50
                sourceSize.height: 50

                fillMode: Image.Stretch
                source: "icons/add.svg"
            }
        }

    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
