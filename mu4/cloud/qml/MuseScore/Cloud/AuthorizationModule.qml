import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

    signal createAccountRequested()
    signal signInRequested()

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundColor
    }

    QtObject {
        id: privateProperties

        readonly property int leftMargin: 134
        readonly property int buttonWidth: 200
    }

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 66
        anchors.leftMargin: privateProperties.leftMargin

        text: qsTrc("scores", "Account")
        font.pixelSize: 32
    }

    Image {
        id: logo

        anchors.top: pageTitle.bottom
        anchors.topMargin: 44

        height: 240
        width: parent.width

        source: "resources/mu_logo_background"

        Image {
            anchors.centerIn: parent
            source: "resources/mu_logo"
        }
    }

    Column {
        anchors.top: logo.bottom
        anchors.left: parent.left
        anchors.topMargin: 76
        anchors.leftMargin: privateProperties.leftMargin

        spacing: 30

        StyledTextLabel {
            text: qsTrc("cloud", "What are the benefits of a MuseScore account?")
            font.pixelSize: 32
        }

        StyledTextLabel {
            text: qsTrc("cloud", "A MuseScore profile allows you to save & publish your scores on MuseScore.com. It's free.")
        }

        ListView {
            spacing: 16
            height: contentHeight
            width: contentWidth

            model: [
                qsTrc("cloud", "Save your scores to private cloud area"),
                qsTrc("cloud", "Share links with other musicians, who add comments"),
                qsTrc("cloud", "Create a portfolio for your music and gain followers"),
                qsTrc("cloud", "Upload high quality audio for superb score playback")
            ]

            delegate: Row {
                spacing: 38

                Rectangle {
                    width: 9
                    height: width
                    color: ui.theme.accentColor
                    radius: width / 2
                    anchors.verticalCenter: parent.verticalCenter
                }

                StyledTextLabel {
                    text: modelData
                }
            }
        }
    }

    FlatButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.bottomMargin: 42
        anchors.leftMargin: privateProperties.leftMargin

        width: privateProperties.buttonWidth
        text: qsTrc("cloud", "Learn more")

        onClicked: {
        }
    }

    Row {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 42
        anchors.rightMargin: privateProperties.leftMargin
        spacing: 22

        FlatButton {
            width: privateProperties.buttonWidth
            text: qsTrc("cloud", "Sign in")
            onClicked: root.signInRequested()
        }

        FlatButton {
            width: privateProperties.buttonWidth
            text: qsTrc("cloud", "Create new account")
            backgroundColor: ui.theme.accentColor
            onClicked: root.createAccountRequested()
        }
    }
}
