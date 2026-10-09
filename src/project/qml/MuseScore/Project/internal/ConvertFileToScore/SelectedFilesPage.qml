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
    property alias saveAsTrimmed: saveAsField.trimmedText
    property alias saveAsErrorText: saveAsField.errorText

    property FileListModel fileListModel: null

    property var fileRequirements: []
    property bool canSelectMultipleFiles: true

    property NavigationSection navigationSection: null

    signal cancelRequested()
    signal backRequested()
    signal selectMoreFilesRequested()
    signal addFilesRequested(var urls)
    signal convertRequested(string convertedScoreName)

    function focusOnDefault() {
        saveAsField.focusOnInput()
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

            navigationPanel: NavigationPanel {
                name: "SelectedFilesPageSaveAs"
                section: root.navigationSection
                order: 2
                enabled: root.enabled && root.visible
            }
            navigationOrder: 0
        }

        ConvertButtonBox {
            Layout.fillWidth: true

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 3

            convertEnabled: Boolean(saveAsField.currentText) && !saveAsField.errorText && !root.fileListModel.exceedsLimits

            onCancelRequested: root.cancelRequested()
            onBackRequested: root.backRequested()

            onConvertRequested: {
                root.convertRequested(saveAsField.trimmedText)
            }
        }
    }

    Component {
        id: multipleFilesPanelComponent

        MultipleFilesPanel {
            navigationPanel: navPanel
            filesModel: root.fileListModel
            fileRequirements: root.fileRequirements

            onSelectMoreFilesRequested: root.selectMoreFilesRequested()

            onFilesDropped: function(urls) {
                root.addFilesRequested(urls)
            }

            onRemoveLastFileRequested: root.backRequested()
        }
    }

    Component {
        id: singleFilePanelComponent

        SingleFilePanel {
            property var item: root.fileListModel.get(0)

            navigationPanel: navPanel
            fileName: item.fileNameRole
            fileSize: item.fileSizeRole
            iconCode: root.fileListModel.fileIconCode
            fileRequirements: root.fileRequirements

            onRemoveRequested: root.backRequested()
        }
    }
}
