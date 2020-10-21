import QtQuick 2.9
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property string title: ""
    property int maxTitleWidth: 0
    property bool isSelected: false
    property bool isMain: false
    property int currentPartIndex: -1

    signal copyPartRequested()
    signal removePartRequested()
    signal partClicked()

    height: 42

    StyledIconLabel {
        id: partIcon

        anchors.left: parent.left

        height: parent.height
        width: height

        iconCode: IconCode.NEW_FILE
    }

    Component {
        id: partTitle

        StyledTextLabel {
            text: root.title

            horizontalAlignment: Qt.AlignLeft
            font.bold: true
        }
    }

    Component {
        id: editPartTitleField

        TextInputField {
            Component.onCompleted: {
                forceActiveFocus()
            }

            currentText: root.title

            onCurrentTextEdited: {
                root.title = newTextValue
            }
        }
    }

    Loader {
        id: titleLoader

        anchors.left: partIcon.right
        anchors.verticalCenter: parent.verticalCenter

        width: root.maxTitleWidth - partIcon.width

        sourceComponent: partTitle

        Connections {
            target: root

            function onCurrentPartIndexChanged(currentPartIndex) {
                titleLoader.sourceComponent = partTitle
            }
        }
    }

    FlatButton {
        id: showVoicesPopupButton

        anchors.left: titleLoader.right
        anchors.verticalCenter: parent.verticalCenter

        normalStateColor: "transparent"
        icon: IconCode.SMALL_ARROW_DOWN

        onClicked: {
            if (voicesPopup.opened) {
                voicesPopup.close()
                return
            }

            voicesPopup.open()
        }
    }

    StyledTextLabel {
        anchors.left: showVoicesPopupButton.right
        anchors.leftMargin: 8
        height: parent.height

        text: voicesPopup.visibleVoicesTitle
        horizontalAlignment: Qt.AlignLeft
    }

    FlatButton {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        normalStateColor: "transparent"
        icon: IconCode.MENU_THREE_DOTS

        onClicked: {
            if (contextMenu.opened) {
                contextMenu.close()
                return
            }

            contextMenu.popup()
        }
    }

    VoicesPopup {
        id: voicesPopup

        x: showVoicesPopupButton.x + showVoicesPopupButton.width / 2 - width / 2
        y: showVoicesPopupButton.y + showVoicesPopupButton.height
    }

    ContextMenu {
        id: contextMenu

        StyledMenuItem {
            text: qsTrc("notation", "Duplicate")

            onTriggered: {
                root.copyPartRequested()
            }
        }

        StyledMenuItem {
            text: qsTrc("notation", "Delete score")

            onTriggered: {
                root.removePartRequested()
            }
        }

        StyledMenuItem {
            text: qsTrc("notation", "Rename")

            onTriggered: {
                titleLoader.sourceComponent = editPartTitleField
            }
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        anchors.leftMargin: -root.anchors.leftMargin
        anchors.rightMargin: -root.anchors.rightMargin

        z: -1

        color: "transparent"
        opacity: 1

        states: [
            State {
                name: "HOVERED"
                when: mouseArea.containsMouse && !mouseArea.pressed && !root.isSelected

                PropertyChanges {
                    target: background
                    opacity: ui.theme.buttonOpacityHover
                    color: ui.theme.buttonColor
                }
            },

            State {
                name: "PRESSED"
                when: mouseArea.pressed && !root.isSelected

                PropertyChanges {
                    target: background
                    opacity: ui.theme.buttonOpacityHit
                    color: ui.theme.buttonColor
                }
            },

            State {
                name: "SELECTED"
                when: root.isSelected

                PropertyChanges {
                    target: background
                    opacity: ui.theme.accentOpacityHit
                    color: ui.theme.accentColor
                }
            }
        ]

        MouseArea {
            id: mouseArea

            anchors.fill: parent

            hoverEnabled: true

            onClicked: {
                root.partClicked()
            }
        }
    }

    SeparatorLine {
        anchors.leftMargin: -root.anchors.leftMargin
        anchors.rightMargin: -root.anchors.rightMargin
        anchors.bottom: parent.bottom
    }
}
