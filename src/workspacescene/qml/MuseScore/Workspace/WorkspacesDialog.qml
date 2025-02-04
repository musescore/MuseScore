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
import MuseScore.Workspace 1.0

import "internal"

StyledDialogView {
    id: root

    contentWidth: 664
    contentHeight: 558
    resizable: true

    WorkspaceListModel {
        id: workspacesModel
    }

    Component.onCompleted: {
        workspacesModel.load()
    }

    onNavigationActivateRequested: {
        view.focusOnSelected()
    }

    onAccessibilityActivateRequested: {
        topPanel.readInfo()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 0

        WorkspacesTopPanel {
            id: topPanel

            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            firstWorkspaceTitle: view.firstWorkspaceTitle
            canRemove: Boolean(workspacesModel.selectedWorkspace) && workspacesModel.selectedWorkspace.isRemovable

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 3

            onCreateNewWorkspaceRequested: {
                workspacesModel.createNewWorkspace()
            }

            onRemoveSelectedWorkspaceRequested: {
                workspacesModel.removeWorkspace(workspacesModel.selectedWorkspace.index)
            }
        }

        SeparatorLine {
            Layout.topMargin: 16
            Layout.leftMargin: -parent.anchors.leftMargin
            Layout.rightMargin: -parent.anchors.rightMargin
        }

        WorkspacesView {
            id: view
            Layout.fillWidth: true
            Layout.preferredHeight: root.height - 190
            Layout.leftMargin: -parent.anchors.leftMargin
            Layout.rightMargin: -parent.anchors.rightMargin
            leftPadding: parent.anchors.leftMargin
            rightPadding: parent.anchors.rightMargin

            model: workspacesModel

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 1
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.topMargin: 20

            buttons: [ ButtonBoxModel.Cancel ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "Select")
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.Select
                accentButton: true

                onClicked: {
                    if (!workspacesModel.apply()) {
                        return
                    }

                    root.ret = { errcode: 0, value: workspacesModel.selectedWorkspace.name }
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
