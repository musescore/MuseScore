import QtQuick 2.9
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Workspace 1.0

import "internal"

QmlDialog {
    id: root

    width: 552
    height: 286

    title: qsTrc("workspace", "Select workspace")

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        QtObject {
            id: privateProperties

            readonly property int sideMargin: 36
            readonly property int buttonsMargin: 24
        }

        WorkspaceListModel {
            id: workspacesModel
        }

        Component.onCompleted: {
            workspacesModel.load()
        }

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height
                Layout.topMargin: privateProperties.sideMargin

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.leftMargin: privateProperties.sideMargin

                    text: qsTrc("workspace", "Workspaces")

                    font.pixelSize: 24
                }

                FlatButton {
                    text: qsTrc("workspace", "Create new workspace")

                    anchors.right: deleteButton.left
                    anchors.rightMargin: 8

                    onClicked: {
                        workspacesModel.createNewWorkspace()
                    }
                }

                FlatButton {
                    id: deleteButton

                    anchors.right: parent.right
                    anchors.rightMargin: privateProperties.buttonsMargin

                    icon: IconCode.DELETE_TANK

                    enabled: Boolean(workspacesModel.selectedWorkspace) && workspacesModel.selectedWorkspace.canRemove

                    onClicked: {
                        workspacesModel.removeWorkspace(workspacesModel.selectedWorkspace.index)
                    }
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                Layout.leftMargin: privateProperties.sideMargin
                Layout.topMargin: 20
                Layout.bottomMargin: 16

                text: qsTrc("workspace", "Use workspaces to save different arrangements of the MuseScore interface")

                horizontalAlignment: Qt.AlignLeft

                font.pixelSize: 12
            }

            SeparatorLine {}

            WorkspacesView {
                id: view

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.bottomMargin: 20

                model: workspacesModel
                sideMargin: privateProperties.sideMargin
            }

            Row {
                Layout.preferredHeight: childrenRect.height
                Layout.rightMargin: privateProperties.buttonsMargin
                Layout.bottomMargin: 16
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                spacing: 12

                FlatButton {
                    text: qsTrc("workspace", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }

                FlatButton {
                    text: qsTrc("workspace", "Select")

                    onClicked: {
                        root.ret = { errcode: 0, value: workspacesModel.selectedWorkspace.name }
                        root.hide()
                    }
                }
            }
        }
    }
}
