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
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui 1.0
import Muse.UiComponents 1.0

RadioButtonGroup {
    id: root

    property string firstWorkspaceTitle: Boolean(prv.selectedWorkspace) ? prv.selectedWorkspace.title : ""

    property int leftPadding: 0
    property int rightPadding: 0

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "WorkspacesViewPanel"
        direction: NavigationPanel.Vertical

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlIndex", prv.currentItemNavigationIndex)
                //! NOTE If we removed workspace, then control with saved index may not be
                event.setData("controlOptional", true)
            }
        }
    }

    spacing: 0
    clip: true
    orientation: Qt.Vertical

    interactive: height < contentHeight

    function focusOnSelected() {
        if (prv.selectedWorkspace) {
            prv.selectedWorkspace.navigation.requestActive()
        }
    }

    QtObject {
        id: prv

        property var currentItemNavigationIndex: []

        property var selectedWorkspace: {
            for (var i = 0; i < root.count; i++) {
                var item = root.itemAtIndex(i)
                if (item && item.isSelected) {
                    return item
                }
            }
        }
    }

    Connections {
        target: root.model

        function onSelectedWorkspaceChanged(selectedWorkspace) {
            if (Boolean(selectedWorkspace)) {
                root.positionViewAtIndex(selectedWorkspace.index, ListView.Contain)
            }
        }
    }

    delegate: ListItemBlank {
        id: listItem

        property string title: model.name
        property string incorrectNameWarning: ""
        property bool isBuiltin: model.isBuiltin

        width: ListView.view.width
        height: 46

        isSelected: model.isSelected

        navigation.panel: root.navigationPanel
        navigation.row: model.index
        navigation.accessible.ignored: true
        navigation.accessible.name: title
        navigation.onActiveChanged: {
            if (!navigation.active) {
                navigation.accessible.ignored = false
            } else {
                positionViewAtIndex(model.index, ListView.Contain)
                prv.currentItemNavigationIndex = [navigation.row, navigation.column]
            }
        }

        function startEditName() {
            if (model.isBuiltin) {
                return
            }

            if (workspaceRadioButton.contentComponent !== editWorkspaceNameField) {
                workspaceRadioButton.contentComponent = editWorkspaceNameField
            }
        }

        function endEditName(newName) {
            if (model.isBuiltin) {
                return
            }

            if (workspaceRadioButton.contentComponent !== workspaceNameLabel) {
                workspaceRadioButton.contentComponent = workspaceNameLabel
                listItem.incorrectNameWarning = ""
                root.model.renameWorkspace(model.index, newName)
                listItem.navigation.requestActive()
            }
        }

        RowLayout {
            anchors.fill: parent

            spacing: 4

            RoundedRadioButton {
                id: workspaceRadioButton

                leftPadding: root.leftPadding
                Layout.fillWidth: true

                ButtonGroup.group: root.radioButtonGroup

                spacing: 12

                checked: model.isSelected

                contentComponent: workspaceNameLabel
                hoverEnabled: false

                onClicked: {
                    root.model.selectWorkspace(model.index)
                }

                onDoubleClicked: {
                    listItem.startEditName()
                }
            }

            FlatButton {
                id: resetButton

                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: root.rightPadding

                text: qsTrc("workspace", "Reset")
                icon: IconCode.UNDO
                orientation: Qt.Horizontal

                navigation.name: "ResetButton"
                navigation.panel: root.navigationPanel
                navigation.column: 2 // todo

                enabled: model.isEdited
                visible: model.isBuiltin

                onClicked: {
                    root.model.resetWorkspace(model.index)
                }
            }
        }

        Component {
            id: workspaceNameLabel

            StyledTextLabel {
                property string accessibleName: text

                text: model.name

                horizontalAlignment: Qt.AlignLeft
                font: model.isSelected ? ui.theme.bodyBoldFont : ui.theme.bodyFont
            }
        }

        Component {
            id: editWorkspaceNameField

            RowLayout {
                property string accessibleName: textField.currentText

                spacing: 38

                TextInputField {
                    id: textField

                    Layout.fillWidth: true

                    navigation.panel: root.navigation.panel
                    navigation.row: root.navigation.row
                    navigation.column: 1

                    maximumLength: 40

                    Component.onCompleted: {
                        forceActiveFocus()
                        navigation.requestActive()
                    }

                    currentText: model.name

                    onTextChanged: function(newTextValue) {
                        listItem.incorrectNameWarning = root.model.validateWorkspaceName(model.index, newTextValue)
                    }

                    onTextEditingFinished: function(newTextValue) {
                        Qt.callLater(listItem.endEditName, newTextValue)
                    }
                }

                StyledTextLabel {
                    Layout.preferredWidth: parent.width / 3

                    text: listItem.incorrectNameWarning

                    horizontalAlignment: Text.AlignLeft
                }
            }
        }

        SeparatorLine { anchors.bottom: parent.bottom }

        onClicked: {
            endEditName()
            root.model.selectWorkspace(model.index)
        }
    }
}
