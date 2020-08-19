import QtQuick 2.7
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

    signal createAccountRequested()
    signal signInRequested()

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    QtObject {
        id: privateProperties

        readonly property int sideMargin: 134
        readonly property int buttonWidth: 200
    }

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 66
        anchors.leftMargin: privateProperties.sideMargin

        text: qsTrc("scores", "Account")

        font.pixelSize: 32
        font.bold: true
    }

    Image {
        id: logo

        anchors.top: pageTitle.bottom
        anchors.topMargin: 44

        height: 240
        width: parent.width

        visible: root.height > 600

        source: "resources/mu_logo_background.jpeg"

        Image {
            anchors.centerIn: parent
            source: "resources/mu_logo.svg"
        }
    }

    AccountBenefitsDescription {
        anchors.top: logo.visible ? logo.bottom : pageTitle.bottom
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin
    }

    Rectangle {
        anchors.bottom: parent.bottom

        height: 114
        width: parent.width

        color: ui.theme.popupBackgroundColor

        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: privateProperties.sideMargin
            anchors.right: parent.right
            anchors.rightMargin: privateProperties.sideMargin

            spacing: 22

            FlatButton {
                Layout.alignment: Qt.AlignLeft

                width: privateProperties.buttonWidth
                text: qsTrc("cloud", "Learn more")

                onClicked: {
                    // TODO: implement me
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 22

                FlatButton {
                    width: privateProperties.buttonWidth
                    text: qsTrc("cloud", "Sign in")

                    onClicked: {
                        root.signInRequested()
                    }
                }

                FlatButton {
                    width: privateProperties.buttonWidth
                    text: qsTrc("cloud", "Create new account")
                    backgroundColor: ui.theme.accentColor

                    onClicked: {
                        root.createAccountRequested()
                    }
                }
            }
        }
    }
}
