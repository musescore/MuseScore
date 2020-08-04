import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

Rectangle {
    id: root

    anchors.fill: parent

    color: ui.theme.backgroundColor

    RowLayout {
        id: topLayout
        anchors.top: parent.top
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 12

        StyledTextLabel {
            id: addonsLabel

            Layout.leftMargin: 133
            Layout.alignment: Qt.AlignLeft

            font.pixelSize: 32
            font.bold: true

            text: qsTrc("appshell", "Add-ons")
        }

        SearchField {
            id: searchField

            Layout.maximumWidth: width
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            width: addonsLabel.width
            Layout.rightMargin: 133
            Layout.alignment: Qt.AlignLeft
        }
    }

    TabBar {
        id: bar

        anchors.top: topLayout.bottom
        anchors.topMargin: 54
        anchors.horizontalCenter: parent.horizontalCenter

        contentHeight: 28
        spacing: 0

        StyledTabButton {
            text: qsTrc("appshell", "Plugins")
            sideMargin: 22
            isCurrent: bar.currentIndex === 0
        }
        StyledTabButton {
            text: qsTrc("appshell", "Extensions")
            sideMargin: 22
            isCurrent: bar.currentIndex === 1
        }
        StyledTabButton {
            text: qsTrc("appshell", "Languages")
            sideMargin: 22
            isCurrent: bar.currentIndex === 2
        }
    }

    StackLayout {
        anchors.top: bar.bottom
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex

        Rectangle {
            id: pluginsComp
            color: ui.theme.backgroundColor
            StyledTextLabel {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Plugins Module"
            }
        }

        ExtensionsContent {
            id: extensionsComp

            Connections {
                target: searchField

                onSearchTextChanged: {
                    extensionsComp.search = searchField.searchText
                }
            }
        }

        Rectangle {
            id: languagesComp
            color: ui.theme.backgroundColor
            StyledTextLabel {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Languages Module"
            }
        }
    }
}

