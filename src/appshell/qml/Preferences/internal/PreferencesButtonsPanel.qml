import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    signal revertFactorySettingsRequested()
    signal applyRequested()
    signal rejectRequested()

    color: ui.theme.backgroundPrimaryColor

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 20

        height: childrenRect.height

        FlatButton {
            anchors.left: parent.left

            width: 160

            text: qsTrc("appshell", "Reset preferences")

            onClicked: {
                root.revertFactorySettingsRequested()
            }
        }

        FlatButton {
            anchors.right: applyButton.left
            anchors.rightMargin: 12

            width: 132
            text: qsTrc("global", "Cancel")

            onClicked: {
                root.rejectRequested()
            }
        }

        FlatButton {
            id: applyButton

            anchors.right: parent.right

            width: 132
            accentButton: true

            text: qsTrc("global", "OK")

            onClicked: {
                root.applyRequested()
            }
        }
    }
}
