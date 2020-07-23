import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Rectangle {
    id: root

    anchors.fill: parent

    color: ui.theme.backgroundColor

    StyledTextLabel {
        id: addonsLabel

        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.left: parent.left
        anchors.leftMargin: 50

        font.pixelSize: 30

        text: qsTrc("appshell", "Add-ons")
    }

    Row {
        id: buttons
        anchors.top: addonsLabel.bottom
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter

        width: 100
        spacing: 10

        FlatButton {
            text: qsTrc("appshell", "Plugins")
            onClicked: {
                load("plugins")
            }
        }
        FlatButton {
            text: qsTrc("appshell", "Extensions")
            onClicked: {
                load("extensions")
            }
        }
        FlatButton {
            text: qsTrc("appshell", "Languages")
            onClicked: {
                load("languages")
            }
        }
    }

    Loader {
        id: loader

        anchors.top: buttons.bottom
        anchors.topMargin: 30
        anchors.left: parent.left
        anchors.leftMargin: 50
        anchors.right: parent.right
        anchors.rightMargin: 15
        anchors.bottom: parent.bottom
    }

    Component {
        id: pluginsComp
        Rectangle {
            color: ui.theme.backgroundColor
            StyledTextLabel {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Plugins Module"
            }
        }
    }

    Component {
        id: extensionsComp
        ExtensionsModule {}
    }

    Component {
        id: languagesComp
        Rectangle {
            color: ui.theme.backgroundColor
            StyledTextLabel {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Languages Module"
            }
        }
    }

    function load(name) {
        switch (name) {
        case "plugins": loader.sourceComponent = pluginsComp; break;
        case "extensions": loader.sourceComponent = extensionsComp; break;
        case "languages": loader.sourceComponent = languagesComp; break;
        }
    }
}

