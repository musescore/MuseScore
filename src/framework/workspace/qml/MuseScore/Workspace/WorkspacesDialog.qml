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

        WorkspaceListModel {
            id: workspacesModel
        }

        Component.onCompleted: {
            workspacesModel.load()
        }

        ColumnLayout {
            readonly property int leftMargin: 36
            readonly property int rightMargin: 16

            anchors.fill: parent
            anchors.topMargin: leftMargin
            anchors.leftMargin: leftMargin
            anchors.rightMargin: rightMargin
            anchors.bottomMargin: rightMargin

            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height

                StyledTextLabel {
                    anchors.left: parent.left

                    text: qsTrc("workspace", "Workspaces")

                    font.pixelSize: 24
                    font.bold: true
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

                    icon: IconCode.DELETE_TANK

                    enabled: Boolean(workspacesModel.selectedWorkspace) && workspacesModel.selectedWorkspace.isRemovable

                    onClicked: {
                        workspacesModel.removeWorkspace(workspacesModel.selectedWorkspace.index)
                    }
                }
            }

            StyledTextLabel {
                Layout.topMargin: 20
                Layout.fillWidth: true

                text: qsTrc("workspace", "Use workspaces to save different arrangements of the MuseScore interface")

                horizontalAlignment: Qt.AlignLeft
            }

            SeparatorLine {
                Layout.topMargin: 16
                Layout.leftMargin: -parent.leftMargin
                Layout.rightMargin: -parent.rightMargin
            }

            WorkspacesView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: -parent.leftMargin
                Layout.rightMargin: -parent.rightMargin
                leftPadding: parent.leftMargin

                model: workspacesModel
            }

            Row {
                Layout.topMargin: 20
                Layout.preferredHeight: childrenRect.height
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

                    enabled: Boolean(workspacesModel.selectedWorkspace)

                    onClicked: {
                        if (!workspacesModel.apply()) {
                            return
                        }

                        root.ret = { errcode: 0, value: workspacesModel.selectedWorkspace.name }
                        root.hide()
                    }
                }
            }
        }
    }
}
