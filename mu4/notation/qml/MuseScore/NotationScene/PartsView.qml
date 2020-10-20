import QtQuick 2.9
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.11

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Column {
    id: root

    property var model

    spacing: 0

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 36

        spacing: 16

        Row {
            width: parent.width
            height: childrenRect.height

            StyledTextLabel {
                width: parent.width / 2

                text: qsTrc("notation", "NAME")

                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase
                font.pixelSize: 12
            }

            StyledTextLabel {
                id: voicesVisibilityLabel

                width: parent.width / 2

                text: qsTrc("notation", "VISIBLE VOICES")

                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase
                font.pixelSize: 12
            }
        }

        SeparatorLine { anchors.margins: -36 }
    }

    ListView {
        id: view

        height: contentHeight
        width: parent.width

        spacing: 0

        model: root.model

        boundsBehavior: Flickable.StopAtBounds
        clip: true

        readonly property int margin: 26

        delegate: Rectangle {
            id: partRect

            width: parent.width
            height: 42

            color: "transparent"
            opacity: 1

            Component {
                id: partTitle

                StyledTextLabel {
                    text: model.title

                    horizontalAlignment: Qt.AlignLeft
                    font.bold: true
                }
            }

            Component {
                id: editPartTitleField

                TextInputField {
                    currentText: model.title

                    onCurrentTextEdited: {
                        root.model.setPartTitle(model.index, newTextValue)
                    }
                }
            }

            Item {
                anchors.fill: parent

                StyledIconLabel {
                    id: partIcon

                    anchors.left: parent.left
                    anchors.leftMargin: 24

                    height: parent.height
                    width: height

                    iconCode: IconCode.NEW_FILE
                }

                Loader {
                    id: titleLoader

                    anchors.left: partIcon.right
                    anchors.verticalCenter: parent.verticalCenter

                    width: voicesVisibilityLabel.x - partIcon.width

                    sourceComponent: partTitle
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

                    text: "All"
                    horizontalAlignment: Qt.AlignLeft
                }

                FlatButton {
                    anchors.right: parent.right
                    anchors.rightMargin: 24
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
            }

            VoicesPopup {
                id: voicesPopup

                x: showVoicesPopupButton.x + showVoicesPopupButton.width / 2 - width / 2
                y: showVoicesPopupButton.y + showVoicesPopupButton.height
            }

            Menu {
                id: contextMenu

                MenuItem {
                    text: qsTrc("notation", "Duplicate")
                }

                MenuItem {
                    text: qsTrc("notation", "Delete score")

                    onTriggered: {
                        root.model.removePart(model.index)
                    }
                }

                MenuItem {
                    text: qsTrc("notation", "Rename")

                    onTriggered: {
                        titleLoader.sourceComponent = editPartTitleField
                    }
                }
            }

            states: [
                State {
                    name: "HOVERED"
                    when: mouseArea.containsMouse && !mouseArea.pressed && !model.isSelected

                    PropertyChanges {
                        target: partRect
                        opacity: ui.theme.buttonOpacityHover
                        color: ui.theme.buttonColor
                    }
                },

                State {
                    name: "PRESSED"
                    when: mouseArea.pressed && !model.isSelected

                    PropertyChanges {
                        target: partRect
                        opacity: ui.theme.buttonOpacityHit
                        color: ui.theme.buttonColor
                    }
                },

                State {
                    name: "SELECTED"
                    when: model.isSelected

                    PropertyChanges {
                        target: partRect
                        opacity: ui.theme.buttonOpacityHit
                        color: ui.theme.accentColor
                    }
                }
            ]

            MouseArea {
                id: mouseArea

                anchors.fill: parent

                hoverEnabled: true

                z: -1

                onClicked: {
                    root.model.selectPart(model.row)
                    mouse.accepted = false
                }
            }

            SeparatorLine { anchors.bottom: parent.bottom }
        }
    }
}
