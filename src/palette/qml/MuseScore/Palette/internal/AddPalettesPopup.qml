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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.12

import MuseScore.Palette 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

StyledPopup {
    id: root

    property PaletteProvider paletteProvider: null
    property int maxHeight: 400

    height: contentColumn.implicitHeight + topPadding + bottomPadding

    navigation.direction: NavigationPanel.Vertical
    navigation.name: "AddPalettesPopup"

    onAboutToShow: {
        palettesList.model = paletteProvider.availableExtraPalettesModel()
    }

    onOpened: {
        createCustomPaletteButton.navigation.requestActive()
    }

    onClosed: {
        palettesList.model = null
    }

    signal addCustomPaletteRequested()

    Column {
        id: contentColumn
        width: parent.width
        spacing: 12

        StyledTextLabel {
            id: header
            text: qsTrc("palette", "More palettes")
            font: ui.theme.bodyBoldFont
        }

        FlatButton {
            id: createCustomPaletteButton
            width: parent.width
            text: qsTrc("palette", "Create custom palette")

            navigation.panel: root.navigation
            navigation.row: 0

            onClicked: {
                root.close();
                root.addCustomPaletteRequested();
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: palettesList.count <= 0
            text: qsTrc("palette", "All palettes were added")
            wrapMode: Text.WordWrap
        }

        ListView {
            id: palettesList
            spacing: 8

            readonly property int availableHeight:
                root.maxHeight - header.height - createCustomPaletteButton.height - 2 * contentColumn.spacing
            height: Math.min(availableHeight, contentHeight)
            width: parent.width

            visible: count > 0

            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: StyledScrollBar {}

            delegate: Item {
                id: morePalettesDelegate

                width: parent.width
                height: addButton.height

                property bool added: false
                property bool removed: false

                Accessible.name: model.display

                RowLayout {
                    anchors.fill: parent
                    spacing: 8
                    visible: !(morePalettesDelegate.added || morePalettesDelegate.removed)

                    StyledTextLabel {
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true

                        height: parent.height
                        text: model.display
                        horizontalAlignment: Text.AlignHLeft

                        wrapMode: Text.WordWrap
                        maximumLineCount: 1
                    }

                    FlatButton {
                        id: addButton
                        Layout.alignment: Qt.AlignRight
                        Layout.preferredWidth: width

                        icon: IconCode.PLUS
                        toolTipTitle: qsTrc("palette", "Add %1 palette").arg(model.display)

                        navigation.panel: root.navigation
                        navigation.row: model.index + 1
                        navigation.onActiveChanged: {
                            if (navigation.active) {
                                palettesList.positionViewAtIndex(model.index, ListView.Contain)
                            }
                        }

                        accessible.name: toolTipTitle

                        onClicked: {
                            if (root.paletteProvider.addPalette(model.paletteIndex)) {
                                morePalettesDelegate.added = true
                            }
                        }
                    }
                }

                StyledTextLabel {
                    anchors.fill: parent

                    visible: morePalettesDelegate.added || morePalettesDelegate.removed
                    text: morePalettesDelegate.added
                          ? qsTrc("palette", "%1 added").arg(model.display)
                          : (morePalettesDelegate.removed ? qsTrc("palette", "%1 removed").arg(model.display) : "")
                    elide: Text.ElideMiddle
                    font: ui.theme.bodyBoldFont
                }
            }
        }
    }
}
