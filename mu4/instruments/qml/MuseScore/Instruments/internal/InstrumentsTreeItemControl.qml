import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    signal clicked()

    RowLayout {
        id: rowLayout

        anchors.fill: root
        anchors.margins: 4

        spacing: 2

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 54 * styleData.depth
            height: childrenRect.height

            FlatButton {
                id: addButton

                anchors.left: parent.left
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
                font.pixelSize: 12
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
