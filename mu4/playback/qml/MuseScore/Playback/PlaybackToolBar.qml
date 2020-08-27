import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.Playback 1.0

Rectangle {

    id: root

    Row {
        anchors.fill: parent

        Repeater {
            anchors.fill: parent
            model: toolModel
            Rectangle {
                id: item

                property bool enabled: enabledRole

                height: parent.height
                width: 60
                color: checkedRole ? "#34C1FF" : root.color
                opacity: item.enabled ? 1.0 : 0.5

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Roboto"
                    font.capitalization: Font.Capitalize
                    text: titleRole
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (item.enabled) {
                            toolModel.click(nameRole)
                        }
                    }
                }
            }
        }
    }

    PlaybackToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }

}
