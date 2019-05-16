import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import "common"
import "style.js" as Style

Window {
//     flags: Qt.FramelessWindowHint
    title: qsTr("Start Center")

    ListModel {
        id: pagesModel
        ListElement {
            name: qsTr("Scores")
            icon: "icons/musical-notes.png"
            content: "ScoresPage.qml"
        }
        ListElement {
            name: qsTr("Extensions")
            icon: "icons/quick-mode.png"
            content: "ExtensionsPage.qml"
        }
        ListElement {
            name: qsTr("Support")
            icon: "icons/support.png"
            content: "SupportPage.qml"
        }
    }

    Item {
        id: leftColumn
        width: parent.width / 4
        height: parent.height

        Rectangle {
            z: -1
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#122535" }
                GradientStop { position: 1.0; color: "#050b10" }
            }
        }

        ItemDelegate {
            id: muLogo
            width: parent.width
            anchors.top: parent.top
            enabled: false
            icon.height: 80
            icon.width: 64
            icon.color: Style.pagesListIconColorDefault
            icon.source: "icons/mu_logo_inverted_128.png"
        }

        ListView {
            id: pagesList
            anchors.top: muLogo.bottom
            anchors.bottom: leftColumnBottomSeparator.top
            width: parent.width
            interactive: false

            model: pagesModel

            delegate: ItemDelegate {
                property string textColor: highlighted ? Style.pagesListTextColorActive : Style.pagesListTextColorDefault
                text: "<font color='" + textColor + "'>"+ model.name + "</font>"
                width: parent.width
                icon.source: model.icon
                highlighted: ListView.isCurrentItem || hovered // TODO: highlight differently? keyboard navigation? activeFocusOnTab?
                icon.color: highlighted ? Style.pagesListIconColorActive : Style.pagesListIconColorDefault
                onClicked: pagesList.currentIndex = index

                background: Rectangle {
                    color: "transparent"
                }
            }
        }

        Rectangle {
            id: leftColumnBottomSeparator
            height: 1
            anchors.left: parent.left
            anchors.leftMargin: Style.layoutMargin
            anchors.right: parent.right
            anchors.rightMargin: Style.layoutMargin
            anchors.bottom: accountLabel.top
            anchors.bottomMargin: Style.accountLabelMargin
            color: Style.borderLinesColor
        }

        ItemDelegate {
            id: accountLabel
            width: parent.width
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Style.accountLabelMargin
            enabled: false
            icon.color: Style.pagesListIconColorDefault
            icon.source: "icons/support.png"
            text: "<font color='" + Style.pagesListTextColorDefault + "'>Ivan Ivanov</font>"
        }
    }

    SwipeView {
        id: pageArea
        anchors.left: leftColumn.right
        anchors.right: parent.right
        height: parent.height
        interactive: false
        orientation: Qt.Vertical

        currentIndex: pagesList.currentIndex

        Repeater {
            model: pagesModel

            delegate: Column {
                StartCenterPage {
                    title: model.name
                    content: model.content
                    height: parent.height
                    width: parent.width
                }
            }
        }
    }
}
