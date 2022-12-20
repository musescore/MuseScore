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

    property string workspaceNames: ""

    contentWidth: 552
    contentHeight: 360

    margins: 24

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
            text: qsTrc("workspace", "Create new workspace")
            font: ui.theme.headerBoldFont
        }

        StyledTextLabel {
            id: workspaceNameTitle
            Layout.topMargin: 24

            text: qsTrc("workspace", "Workspace name:")
        }

        TextInputField {
            id: workspaceNameField
            Layout.topMargin: 12
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
            Layout.topMargin: 12
            Layout.fillWidth: true

            horizontalAlignment: Qt.AlignLeft
            text: workspaceModel.errorMessage
        }

        StyledTextLabel {
            id: selectOptionsLabel

            Layout.topMargin: 24
            Layout.fillWidth: true

            text: qsTrc("workspace", "Select the options you want remembered in your new workspace")

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

                text: qsTrc("workspace", "UI preferences (colors, canvas style, etc.)")

                navigation.name: "UIPreferencesCheckBox"
                navigation.panel: content.navigationPanel
                navigation.row: 2
                navigation.accessible.name: selectOptionsLabel.text + " " + text

                onClicked: {
                    workspaceModel.useUiPreferences = !checked
                }
            }

            CheckBox {
                checked: workspaceModel.useUiArrangement

                text: qsTrc("workspace", "UI arrangement")

                navigation.name: "UIPreferencesCheckBox"
                navigation.panel: content.navigationPanel
                navigation.row: 3

                onClicked: {
                    workspaceModel.useUiArrangement = !checked
                }
            }

            CheckBox {
                checked: workspaceModel.usePalettes

                text: qsTrc("workspace", "Palettes")

                navigation.name: "UIPreferencesCheckBox"
                navigation.panel: content.navigationPanel
                navigation.row: 4

                onClicked: {
                    workspaceModel.usePalettes = !checked
                }
            }

            CheckBox {
                checked: workspaceModel.useToolbarCustomization

                text: qsTrc("workspace", "Toolbar customizations")

                navigation.name: "UIPreferencesCheckBox"
                navigation.panel: content.navigationPanel
                navigation.row: 5

                onClicked: {
                    workspaceModel.useToolbarCustomization = !checked
                }
            }
        }

        NewWorkspaceBottomPanel {
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            Layout.topMargin: 42

            canCreateWorkspace: workspaceModel.isWorkspaceNameAllowed

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            onCancelRequested: {
                root.reject()
            }

            onSelectRequested: {
                root.ret = { errcode: 0, value: workspaceModel.createWorkspace() }
                root.hide()
            }
        }
    }
}
