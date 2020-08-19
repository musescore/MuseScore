import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

    property alias userName: userName.text
    property alias avatarUrl: accountAvatar.url
    property string profileUrl: ""
    property string sheetmusicUrl: ""

    signal signOutRequested()

    QtObject {
        id: privateProperties

        readonly property int sideMargin: 134
        readonly property int buttonWidth: 134
    }

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    StyledTextLabel {
        id: userName

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 66
        anchors.leftMargin: privateProperties.sideMargin

        font.pixelSize: 32
        font.bold: true
    }

    Row {
        anchors.top: userName.bottom
        anchors.topMargin: 106
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin

        width: parent.width
        spacing: 67

        AccountAvatar {
            id: accountAvatar

            side: 200
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 20

            StyledTextLabel {
                text: qsTrc("cloud", "Your profile link:")
                font.pixelSize: 18
            }

            StyledTextLabel {
                text: "MuseScore.com/" + root.userName
                font.pixelSize: 18
                color: ui.theme.accentColor

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        api.launcher.openUrl(root.sheetmusicUrl)
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom

        height: 114
        width: parent.width

        color: ui.theme.popupBackgroundColor

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 134

            spacing: 22

            FlatButton {
                width: privateProperties.buttonWidth
                backgroundColor: ui.theme.accentColor
                text: qsTrc("cloud", "Account info")

                onClicked: {
                    api.launcher.openUrl(root.profileUrl)
                }
            }

            FlatButton {
                width: privateProperties.buttonWidth
                text: qsTrc("cloud", "Sign out")

                onClicked: {
                    root.signOutRequested()
                }
            }
        }
    }
}
