/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

Item {
    id: root

    property var files: []
    property string link: ""
    property bool canSelectMultipleFiles: true
    property var fileRequirements: []
    property NavigationSection navigationSection: null

    readonly property string saveAsNameError: fileListModel.validateFileName(saveAsField.currentText)

    signal cancelRequested()
    signal backRequested(bool confirm)
    signal selectMoreFilesRequested(var existingPaths)
    signal convertRequested(var paths, string link, string convertedFileName)

    function focusOnFileList() {
        if (filesPanelLoader.item) {
            filesPanelLoader.item.focusOnFileList()
        }
    }

    onFilesChanged: {
        var wasEmpty = fileListModel.count === 0
        fileListModel.setPaths(root.files)
        if (wasEmpty && fileListModel.count > 0) {
            saveAsField.currentText = fileListModel.defaultSaveAsName()
        }
    }

    FileListModel {
        id: fileListModel

        onCountChanged: {
            if (fileListModel.count === 0) {
                root.backRequested(false /*confirm*/)
            }
        }
    }

    NavigationPanel {
        id: navPanel
        name: "SelectedFilesPage"
        section: root.navigationSection
        order: 1
        enabled: root.enabled && root.visible
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 18

        Loader {
            id: filesPanelLoader

            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: {
                if (root.link) {
                    return linkPanelComponent
                }

                return root.canSelectMultipleFiles ? multipleFilesPanelComponent : singleFilePanelComponent
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 8

            StyledTextLabel {
                text: qsTrc("project/convert", "Save as")
                horizontalAlignment: Text.AlignLeft
            }

            TextInputField {
                id: saveAsField

                navigation.panel: navPanel
                navigation.order: 5

                onTextChanged: function(newTextValue) {
                    saveAsField.currentText = newTextValue
                }
            }

            StyledTextLabel {
                width: parent.width

                visible: Boolean(text)
                horizontalAlignment: Text.AlignLeft
                text: root.saveAsNameError
            }
        }

        ButtonBox {
            Layout.fillWidth: true

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "Cancel")

                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.CustomButton + 1
                isLeftSide: true

                onClicked: {
                    root.cancelRequested()
                }
            }

            FlatButton {
                text: qsTrc("global", "Back")

                buttonRole: ButtonBoxModel.BackRole
                buttonId: ButtonBoxModel.CustomButton + 2

                onClicked: {
                    root.backRequested(true /*confirm*/)
                }
            }

            FlatButton {
                text: qsTrc("project/convert", "Convert")

                buttonRole: ButtonBoxModel.ApplyRole
                buttonId: ButtonBoxModel.CustomButton + 3

                accentButton: true
                enabled: Boolean(saveAsField.currentText) && !root.saveAsNameError

                onClicked: {
                    root.convertRequested(root.link ? [] : fileListModel.paths(), root.link, saveAsField.currentText)
                }
            }
        }
    }

    Component {
        id: multipleFilesPanelComponent

        MultipleFilesPanel {
            navigationPanel: navPanel
            filesModel: fileListModel
            fileRequirements: root.fileRequirements

            onSelectMoreFilesRequested: function(existingPaths) {
                root.selectMoreFilesRequested(existingPaths)
            }
        }
    }

    Component {
        id: linkPanelComponent

        Rectangle {
            function focusOnFileList() {}

            color: ui.theme.backgroundSecondaryColor
            border.width: 1
            border.color: ui.theme.strokeColor
            radius: 3

            StyledTextLabel {
                anchors.centerIn: parent
                anchors.margins: 16
                width: parent.width - 32

                text: root.link
                wrapMode: Text.Wrap
            }
        }
    }

    Component {
        id: singleFilePanelComponent

        SingleFilePanel {
            navigationPanel: navPanel
            fileName: fileListModel.fileName(0)
            iconCode: fileListModel.fileIconCode
            fileRequirements: root.fileRequirements

            onRemoveRequested: {
                fileListModel.removeAt(0)
            }
        }
    }
}
