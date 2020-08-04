import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

Item {
    id: root

    height: 40

    property string userName: ""
    property string avatarUrl: ""

    signal clicked()

    Row {
        anchors.fill: parent
        anchors.leftMargin: 18

        spacing: 23

        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter
            height: root.height
            iconCode: IconCode.ACCOUNT
            visible: !Boolean(root.avatarUrl)
        }

        AccountAvatar {
            url: root.avatarUrl
            side: 40
            visible: Boolean(root.avatarUrl)
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter
            text: Boolean(root.userName) ? root.userName : qsTrc("cloud", "My Account")
            font.pixelSize: 18
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
