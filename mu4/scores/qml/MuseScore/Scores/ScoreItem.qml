import QtQuick 2.7
import MuseScore.Ui 1.0
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

        StyledTextLabel {
            id: scoreTitle

            anchors.horizontalCenter: parent.horizontalCenter

            text: title
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

            StyledIconLabel {
                anchors.centerIn: parent
                height: 50
                width: height

                iconCode: IconCode.PLUS
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
