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

Item {
    id: root

    property string guidelinesUrl: ""
    property alias linkHintText: convertFromLinkPanel.hintText
    property var fileRequirements: []

    property NavigationSection navigationSection: null

    readonly property int contentPadding: 24

    signal selectFilesRequested()
    signal filesDropped(var urls)
    signal convertFromLinkRequested()
    signal cancelRequested()

    function focusOnSelect() {
        selectFileButton.navigation.requestActive()
    }

    NavigationPanel {
        id: navPanel
        name: "SelectFilePage"
        section: root.navigationSection
        order: 1
        enabled: root.enabled && root.visible
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 18

        Item {
            id: dropZone

            Layout.fillWidth: true
            Layout.fillHeight: true

            Canvas {
                id: dashedBorder

                anchors.fill: parent

                function tracePath(ctx, inset) {
                    var radius = 3
                    var w = width - inset * 2
                    var h = height - inset * 2
                    ctx.beginPath()
                    ctx.moveTo(inset + radius, inset)
                    ctx.lineTo(inset + w - radius, inset)
                    ctx.quadraticCurveTo(inset + w, inset, inset + w, inset + radius)
                    ctx.lineTo(inset + w, inset + h - radius)
                    ctx.quadraticCurveTo(inset + w, inset + h, inset + w - radius, inset + h)
                    ctx.lineTo(inset + radius, inset + h)
                    ctx.quadraticCurveTo(inset, inset + h, inset, inset + h - radius)
                    ctx.lineTo(inset, inset + radius)
                    ctx.quadraticCurveTo(inset, inset, inset + radius, inset)
                }

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    ctx.fillStyle = ui.theme.backgroundSecondaryColor
                    tracePath(ctx, 0)
                    ctx.fill()

                    ctx.strokeStyle = ui.theme.strokeColor
                    ctx.lineWidth = 1
                    ctx.setLineDash([4, 4])
                    tracePath(ctx, 0.5)
                    ctx.stroke()
                }

                Component.onCompleted: requestPaint()
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
            }

            ColumnLayout {
                anchors.centerIn: parent

                width: parent.width - 2 * root.contentPadding
                spacing: 6

                StyledIconLabel {
                    Layout.alignment: Qt.AlignHCenter

                    iconCode: IconCode.SHARE_FILE
                    font.pixelSize: 36
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 2

                    text: qsTrc("project/convert", "Drag your file here")
                    font: ui.theme.tabBoldFont
                }

                StyledTextLabel {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 12

                    text: qsTrc("project/convert", "Use a PDF, images, or MP3 (beta)")
                    font: ui.theme.bodyFont
                }

                FileRequirements {
                    Layout.alignment: Qt.AlignHCenter

                    fileRequirements: root.fileRequirements

                    navigation.panel: navPanel
                    navigation.order: 1
                }

                FlatButton {
                    id: selectFileButton

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 18

                    text: qsTrc("global", "Select")
                    accentButton: true

                    navigation.panel: navPanel
                    navigation.order: 0

                    onClicked: {
                        root.selectFilesRequested()
                    }
                }
            }

            DropArea {
                anchors.fill: parent

                onDropped: function(drop) {
                    if (drop.hasUrls) {
                        var urls = drop.urls.map(function(url) { return url.toString() })
                        root.filesDropped(urls)
                    }
                }
            }
        }

        ConvertFromLinkPanel {
            id: convertFromLinkPanel

            Layout.fillWidth: true
            Layout.topMargin: 2

            navigationPanel: navPanel
            navigationOrder: 2

            onConvertFromLinkRequested: {
                root.convertFromLinkRequested()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Item {
                id: guidelinesItem

                Layout.fillWidth: true
                implicitHeight: guidelinesLabel.implicitHeight

                NavigationControl {
                    id: guidelinesNavCtrl

                    name: "UploadingGuidelines"
                    panel: navPanel
                    order: 3
                    enabled: root.enabled && root.visible

                    accessible.role: MUAccessible.Button
                    accessible.name: qsTrc("project/convert", "Uploading guidelines")
                    accessible.visualItem: guidelinesItem

                    onTriggered: {
                        Qt.openUrlExternally(root.guidelinesUrl)
                    }
                }

                NavigationFocusBorder {
                    navigationCtrl: guidelinesNavCtrl
                }

                StyledTextLabel {
                    id: guidelinesLabel

                    anchors.fill: parent

                    text: "<a href=\"" + root.guidelinesUrl + "\">" + qsTrc("project/convert", "Uploading guidelines") + "</a>"
                    font: ui.theme.bodyFont
                    horizontalAlignment: Text.AlignLeft
                }
            }

            ButtonBox {
                buttons: [ButtonBoxModel.Cancel]

                navigationPanel.section: root.navigationSection
                navigationPanel.order: 4

                onStandardButtonClicked: function(buttonId) {
                    if (buttonId === ButtonBoxModel.Cancel) {
                        root.cancelRequested()
                    }
                }
            }
        }
    }
}
