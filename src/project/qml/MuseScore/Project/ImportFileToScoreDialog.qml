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

StyledDialogView {
    id: root

    title: qsTrc("project", "Import file to Score")

    contentWidth: 520
    contentHeight: 362
    margins: 24

    modal: true

    ImportFileToScoreModel {
        id: importModel
    }

    function finish(files) {
        root.ret = { errcode: 0, value: files }
        root.hide()
    }

    onNavigationActivateRequested: {
        chooseFileButton.navigation.requestActive()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 24

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            text: qsTrc("project", "Convert from a file to MSCZ")
            font: ui.theme.largeBodyBoldFont
        }

        Item {
            id: dropZone

            Layout.fillWidth: true
            Layout.fillHeight: true

            Canvas {
                id: dashedBorder

                anchors.fill: parent

                onPaint: {
                    var radius = 8
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.strokeStyle = ui.theme.strokeColor
                    ctx.lineWidth = 1
                    ctx.setLineDash([4, 4])
                    ctx.beginPath()
                    ctx.moveTo(radius, 0)
                    ctx.lineTo(width - radius, 0)
                    ctx.quadraticCurveTo(width, 0, width, radius)
                    ctx.lineTo(width, height - radius)
                    ctx.quadraticCurveTo(width, height, width - radius, height)
                    ctx.lineTo(radius, height)
                    ctx.quadraticCurveTo(0, height, 0, height - radius)
                    ctx.lineTo(0, radius)
                    ctx.quadraticCurveTo(0, 0, radius, 0)
                    ctx.stroke()
                }

                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
            }

            ColumnLayout {
                anchors.centerIn: parent

                width: parent.width - 48
                spacing: 12

                StyledIconLabel {
                    Layout.alignment: Qt.AlignHCenter

                    iconCode: IconCode.IMPORT
                    font.pixelSize: 32
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignHCenter

                    text: qsTrc("project", "Drag and drop your file here")
                    font: ui.theme.tabBoldFont
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignHCenter

                    text: qsTrc("project", "Acceptable files - .pdf .mp3 (beta) .jpg .png")
                    font: ui.theme.bodyFont
                    opacity: 0.7
                }

                FlatButton {
                    id: chooseFileButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 8

                    text: qsTrc("project", "Choose file")
                    accentButton: true

                    navigation.panel: cancelButtonBox.navigationPanel
                    navigation.order: 1

                    onClicked: {
                        var files = importModel.selectFiles()
                        if (files.length > 0) {
                            root.finish(files)
                        }
                    }
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 8

                    visible: importModel.errorMessage.length > 0

                    text: importModel.errorMessage
                    font: ui.theme.bodyFont
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 16

                    text: "<a href=\"https://musescore.com/score-uploading-guidelines\">" + qsTrc("project", "Uploading guidelines") + "</a>"
                    font: ui.theme.bodyFont
                }
            }

            DropArea {
                anchors.fill: parent

                onDropped: function(drop) {
                    if (drop.hasUrls) {
                        var urls = drop.urls.map(function(url) { return url.toString() })
                        var files = importModel.localPaths(urls)
                        if (importModel.checkFiles(files)) {
                            root.finish(files)
                        }
                    }
                }
            }
        }

        ButtonBox {
            id: cancelButtonBox

            Layout.fillWidth: true

            buttons: [ButtonBoxModel.Cancel]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    root.reject()
                }
            }
        }
    }
}
