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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            StyledTextLabel {
                anchors.left: parent.left

                text: qsTrc("workspace", "Workspaces")
                font: ui.theme.headerBoldFont
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
            Layout.leftMargin: -parent.anchors.leftMargin
            Layout.rightMargin: -parent.anchors.rightMargin
        }

        WorkspacesView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: -parent.anchors.leftMargin
            Layout.rightMargin: -parent.anchors.rightMargin
            leftPadding: parent.anchors.leftMargin

            model: workspacesModel
        }

        Row {
            Layout.topMargin: 20
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
