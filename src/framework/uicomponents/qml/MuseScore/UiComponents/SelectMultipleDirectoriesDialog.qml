import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    width: 664
    height: 300

    property string initiallyDirectories: ""

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        SelectMultipleDirectoriesModel {
            id: directoriesModel
        }

        QtObject {
            id: privateProperties

            readonly property int sideMargin: 36
            readonly property int buttonsMargin: 24
        }

        Component.onCompleted: {
            directoriesModel.load(root.initiallyDirectories)
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 24

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: privateProperties.sideMargin
                Layout.leftMargin: privateProperties.sideMargin
                Layout.rightMargin: privateProperties.buttonsMargin
                spacing: 8

                StyledTextLabel {
                    Layout.fillWidth: true

                    text: qsTrc("uicomponents", "Directories")
                    font: ui.theme.headerBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight

                    text: qsTrc("uicomponents", "Add directory")

                    onClicked: {
                        directoriesModel.addDirectory()
                    }
                }

                FlatButton {
                    Layout.alignment: Qt.AlignRight

                    icon: IconCode.DELETE_TANK

                    enabled: directoriesModel.selection.hasSelection

                    onClicked: {
                        directoriesModel.deleteSelectedDirectories()
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true

                color: ui.theme.backgroundPrimaryColor
                border.width: 1
                border.color: ui.theme.strokeColor

                ListView {
                    id: view

                    anchors.fill: parent
                    anchors.margins: 1

                    model: directoriesModel

                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: StyledScrollBar {
                        parent: view.parent

                        Layout.alignment: Qt.AlignTop | Qt.AlignBottom | Qt.AlignRight
                        Layout.rightMargin: 16

                        visible: view.contentHeight > view.height
                        z: 1
                    }

                    delegate: ListItemBlank {

                        normalStateColor: (index % 2 == 0) ? ui.theme.backgroundSecondaryColor
                                                           : ui.theme.backgroundPrimaryColor

                        isSelected: selectedRole

                        onClicked: {
                            directoriesModel.selectRow(index)
                        }

                        StyledTextLabel {
                            anchors.fill: parent
                            anchors.leftMargin: 24
                            anchors.rightMargin: 24

                            text: titleRole
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            }

            Row {
                Layout.preferredHeight: childrenRect.height
                Layout.bottomMargin: 24
                Layout.rightMargin: 24
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                spacing: 12

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "OK")

                    onClicked: {
                        root.ret = {errcode: 0, value: directoriesModel.directories()}
                        root.hide()
                    }
                }
            }
        }
    }
}
