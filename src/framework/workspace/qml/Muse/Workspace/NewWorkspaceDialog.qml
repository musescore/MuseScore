/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Workspace 1.0

import "internal"

StyledDialogView {
    id: root

    property string workspaceNames: ""

    contentWidth: 552
    contentHeight: 118

    margins: 12

    NewWorkspaceModel {
        id: workspaceModel
    }

    Component.onCompleted: {
        workspaceModel.load(root.workspaceNames)
    }

    onNavigationActivateRequested: {
        workspaceNameField.navigation.requestActive()
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        spacing: 0

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "NewWorkspacePanel"
            direction: NavigationPanel.Vertical
            section: root.navigationSection
            order: 1
        }

        StyledTextLabel {
            id: workspaceNameTitle

            text: qsTrc("workspace", "Workspace name:")
        }

        TextInputField {
            id: workspaceNameField
            Layout.topMargin: 4
            Layout.fillWidth: true

            currentText: workspaceModel.workspaceName

            navigation.name: "WorkspaceNameInputField"
            navigation.panel: content.navigationPanel
            navigation.row: 1
            navigation.accessible.name: workspaceNameTitle.text + " " + currentText

            maximumLength: 40

            onTextChanged: function(newTextValue) {
                workspaceModel.workspaceName = newTextValue
            }

            Component.onCompleted: {
                selectAll()
            }
        }

        StyledTextLabel {
            Layout.topMargin: 4
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft
            text: workspaceModel.errorMessage
        }

        SeparatorLine {
            Layout.topMargin: 4
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.topMargin: 12

            buttons: [ ButtonBoxModel.Cancel ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "Create")
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.CustomButton + 1
                accentButton: true
                enabled: workspaceModel.isWorkspaceNameAllowed

                onClicked: {
                    root.ret = { errcode: 0, value: workspaceModel.createWorkspace() }
                    root.hide()
                }
            }

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    root.reject()
                }
            }
        }
    }
}
