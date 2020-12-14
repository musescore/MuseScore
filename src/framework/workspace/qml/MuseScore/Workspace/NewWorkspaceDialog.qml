import QtQuick 2.12
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Workspace 1.0

QmlDialog {
    id: root

    width: 552
    height: 360

    Rectangle {
        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        NewWorkspaceModel {
            id: workspaceModel
        }

        Component.onCompleted: {
            workspaceModel.load()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24

            spacing: 0

            StyledTextLabel {
                text: qsTrc("workspace", "Create new workspace")
                font: ui.theme.headerBoldFont
            }

            StyledTextLabel {
                Layout.topMargin: 24

                text: qsTrc("workspace", "Workspace name:")
            }

            TextInputField {
                Layout.topMargin: 12
                Layout.fillWidth: true

                currentText: workspaceModel.workspaceName

                onCurrentTextEdited: {
                    workspaceModel.workspaceName = newTextValue
                }

                Component.onCompleted: {
                    selectAll()
                }
            }

            StyledTextLabel {
                Layout.topMargin: 24
                Layout.fillWidth: true

                text: qsTrc("workspace", "Select the options you want remembered in your new Workspace")

                horizontalAlignment: Qt.AlignLeft
            }

            Grid {
                Layout.topMargin: 20
                Layout.preferredHeight: childrenRect.height
                Layout.fillWidth: true

                columns: 2
                rowSpacing: 20
                columnSpacing: rowSpacing * 4

                CheckBox {
                    checked: workspaceModel.importUiPreferences

                    text: qsTrc("workspace", "UI preferences (colours, canvas style, etc.)")

                    onClicked: {
                        workspaceModel.importUiPreferences = !checked
                    }
                }

                CheckBox {
                    checked: workspaceModel.importUiArrangement

                    text: qsTrc("workspace", "UI arrangement")

                    onClicked: {
                        workspaceModel.importUiArrangement = !checked
                    }
                }

                CheckBox {
                    checked: workspaceModel.importPalettes

                    text: qsTrc("workspace", "Palettes")

                    onClicked: {
                        workspaceModel.importPalettes = !checked
                    }
                }

                CheckBox {
                    checked: workspaceModel.importToolbarCustomization

                    text: qsTrc("workspace", "Toolbar customisations")

                    onClicked: {
                        workspaceModel.importToolbarCustomization = !checked
                    }
                }
            }

            Row {
                Layout.topMargin: 42
                Layout.preferredHeight: childrenRect.height
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                spacing: 12

                FlatButton {
                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Select")

                    enabled: workspaceModel.canCreateWorkspace

                    onClicked: {
                        root.ret = { errcode: 0, value: workspaceModel.createWorkspace() }
                        root.hide()
                    }
                }
            }
        }
    }
}
