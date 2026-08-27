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
import Muse.Cloud
import MuseScore.Project

import "internal/ConvertFileToScore"

StyledDialogView {
    id: root

    title: qsTrc("project/convert", "Convert file to score")

    contentHeight: 502
    contentWidth: 616
    margins: 0

    modal: true
    resizable: false

    background.color: ui.theme.popupBackgroundColor

    property int convertType: -1
    property int currentPageIndex: 0
    property var selectedPaths: []
    property string selectedLink: ""

    ConvertFileToScoreModel {
        id: convertModel

        onValidationFinished: function(type, paths) {
            root.convertType = type
            root.selectedPaths = paths
            root.selectedLink = ""

            if (root.currentPageIndex === 0) {
                root.currentPageIndex = 1
            }
        }

        onGoingBackConfirmed: {
            root.currentPageIndex = 0
            root.selectedLink = ""
        }
    }

    function finish(type, paths, link, convertedFileName) {
        root.ret = { errcode: 0, value: { type: type, paths: paths, link: link, convertedFileName: convertedFileName } }
        root.hide()
    }

    function selectAndValidateFiles(existingPaths) {
        var files = convertModel.selectFiles(existingPaths)
        if (files.length > 0) {
            convertModel.validateFiles(existingPaths.concat(files))
        }
    }

    onNavigationActivateRequested: {
        if (root.currentPageIndex === 0) {
            pageLoader.item.focusOnSelect()
        } else {
            pageLoader.item.focusOnFileList()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 18
        anchors.bottomMargin: 18
        anchors.leftMargin: 48
        anchors.rightMargin: 48
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            StyledTextLabel {
                Layout.fillWidth: true

                text: qsTrc("project/convert", "Convert a file to score")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            AccountAvatar {
                side: 30
                withBackground: false
                url: convertModel.accountAvatarUrl
            }
        }

        Loader {
            id: pageLoader

            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: root.currentPageIndex === 0 ? selectFilePageComponent : selectedFilesPageComponent
        }
    }

    Component {
        id: selectFilePageComponent

        SelectFilePage {
            guidelinesLinkText: convertModel.guidelinesLinkText
            linkPasteText: convertModel.linkPasteText
            maxLinkLength: convertModel.maxLinkLength
            fileRequirements: convertModel.fileRequirements
            navigationSection: root.navigationSection

            onCancelRequested: root.reject()

            onSelectFilesRequested: root.selectAndValidateFiles([])

            onFilesDropped: function(urls) {
                convertModel.validateFiles(urls)
            }

            onLinkSubmitted: function(link) {
                root.convertType = 1 // audio2score
                root.selectedPaths = []
                root.selectedLink = link
                root.currentPageIndex = 1
            }
        }
    }

    Component {
        id: selectedFilesPageComponent

        SelectedFilesPage {
            navigationSection: root.navigationSection
            files: root.selectedPaths
            link: root.selectedLink
            canSelectMultipleFiles: convertModel.canSelectMultipleFiles(root.convertType, root.selectedPaths)
            fileRequirements: convertModel.fileRequirements

            onCancelRequested: root.reject()

            onBackRequested: function(confirm) {
                if (confirm) {
                    convertModel.confirmGoingBack()
                } else {
                    root.currentPageIndex = 0
                    root.selectedLink = ""
                }
            }

            onConvertRequested: function(paths, link, convertedFileName) {
                root.finish(root.convertType, paths, link, convertedFileName)
            }

            onSelectMoreFilesRequested: function(existingPaths) {
                root.selectAndValidateFiles(existingPaths)
            }
        }
    }
}
