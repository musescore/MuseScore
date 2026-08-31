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

    property alias saveAsName: saveAsField.currentText
    property alias saveAsErrorText: saveAsField.errorText

    property var files: []

    property var convertLimits: ({})
    property var fileRequirements: []
    property bool canSelectMultipleFiles: true

    property NavigationSection navigationSection: null

    signal cancelRequested()
    signal backRequested(bool confirm)
    signal selectMoreFilesRequested(var existingPaths)
    signal convertRequested(var paths, string convertedFileName)

    function focusOnFileList() {
        if (filesPanelLoader.item) {
            filesPanelLoader.item.focusOnFileList()
        }
    }

    FileListModel {
        id: fileListModel

        paths: root.files
        convertLimits: root.convertLimits

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

            sourceComponent: root.canSelectMultipleFiles ? multipleFilesPanelComponent : singleFilePanelComponent
        }

        SaveAsField {
            id: saveAsField

            Layout.fillWidth: true

            navigationPanel: navPanel
            navigationOrder: 5
        }

        ConvertButtonBox {
            Layout.fillWidth: true

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            convertEnabled: Boolean(saveAsField.currentText) && !saveAsField.errorText && !fileListModel.exceedsLimits

            onCancelRequested: root.cancelRequested()
            onBackRequested: root.backRequested(true /*confirm*/)

            onConvertRequested: {
                root.convertRequested(fileListModel.paths, saveAsField.currentText)
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
        id: singleFilePanelComponent

        SingleFilePanel {
            property var item: fileListModel.get(0)

            navigationPanel: navPanel
            fileName: item.fileNameRole
            fileSize: item.fileSizeRole
            iconCode: fileListModel.fileIconCode
            fileRequirements: root.fileRequirements

            onRemoveRequested: {
                fileListModel.removeAt(0)
            }
        }
    }
}
