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

    signal requestActiveFocus()

    contentWidth: 552
    contentHeight: 286

    WorkspaceListModel {
        id: workspacesModel
    }

    Component.onCompleted: {
        workspacesModel.load()
    }

    NavigationSection {
        id: navTopSec
        name: "WorkspacesTop"
        enabled: root.visible
        order: 10
        onActiveChanged: {
            if (active) {
                root.requestActiveFocus()
            }
        }
    }

    NavigationSection {
        id: navWorkspacesSec
        name: "Workspaces"
        enabled: root.visible
        order: 11
        onActiveChanged: {
            if (active) {
                root.requestActiveFocus()
            }
        }
    }

    NavigationSection {
        id: navBottomSec
        name: "WorkspacesBottom"
        enabled: root.visible
        order: 12
        onActiveChanged: {
            if (active) {
                root.requestActiveFocus()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 0

        NavigationPanel {
            id: navTopPanel
            name: "Workspaces Top"
            section: navTopSec
            direction: NavigationPanel.Horizontal
            order: 1
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            StyledTextLabel {
                anchors.left: parent.left

                text: qsTrc("workspace", "Workspaces")
                font: ui.theme.headerBoldFont
            }

            FlatButton {
                navigation.name: "New Workspace"
                navigation.panel: navTopPanel
                navigation.column: 1

                text: qsTrc("workspace", "Create new workspace")

                anchors.right: deleteButton.left
                anchors.rightMargin: 8

                onClicked: {
                    workspacesModel.createNewWorkspace()
                }
            }

            FlatButton {
                navigation.name: "Delete Workspace"
                navigation.panel: navTopPanel
                navigation.column: 2

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

            navigation.section: navWorkspacesSec
            navigation.order: 1

            model: workspacesModel
        }

        NavigationPanel {
            id: navBottomPanel
            name: "Workspaces Bottom"
            section: navBottomSec
            direction: NavigationPanel.Horizontal
            order: 1
        }

        Row {
            Layout.topMargin: 20
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            spacing: 12

            FlatButton {
                navigation.name: "Cancel"
                navigation.panel: navBottomPanel
                navigation.column: 1

                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                navigation.name: "Select"
                navigation.panel: navBottomPanel
                navigation.column: 2
                
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
