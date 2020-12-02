import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    color: ui.theme.textFieldColor

    signal clicked()

    RowLayout {
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.bottom: separator.top
        anchors.bottomMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 2

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 90 * styleData.depth
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                id: addButton

                anchors.left: parent.left

                width: 24
                height: width

                icon: IconCode.PLUS
            }

            StyledTextLabel {
                anchors {
                    left: addButton.right
                    leftMargin: 8
                    right: parent.right
                    rightMargin: 8
                    verticalCenter: addButton.verticalCenter
                }
                horizontalAlignment: Text.AlignLeft

                text: model ? model.itemRole.title : ""
            }
        }
    }

    SeparatorLine { id: separator; anchors.bottom: parent.bottom; }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
