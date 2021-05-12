/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Workspace 1.0

StyledDialogView {
    id: root

    contentWidth: 552
    contentHeight: 360

    margins: 24

    NewWorkspaceModel {
        id: workspaceModel
    }

    Component.onCompleted: {
        workspaceModel.load()
    }

    ColumnLayout {
        anchors.fill: parent
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
                checked: workspaceModel.useUiPreferences

                text: qsTrc("workspace", "UI preferences (colours, canvas style, etc.)")

                onClicked: {
                    workspaceModel.useUiPreferences = !checked
                }
            }

            CheckBox {
                checked: workspaceModel.useUiArrangement

                text: qsTrc("workspace", "UI arrangement")

                onClicked: {
                    workspaceModel.useUiArrangement = !checked
                }
            }

            CheckBox {
                checked: workspaceModel.usePalettes

                text: qsTrc("workspace", "Palettes")

                onClicked: {
                    workspaceModel.usePalettes = !checked
                }
            }

            CheckBox {
                checked: workspaceModel.useToolbarCustomization

                text: qsTrc("workspace", "Toolbar customisations")

                onClicked: {
                    workspaceModel.useToolbarCustomization = !checked
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
