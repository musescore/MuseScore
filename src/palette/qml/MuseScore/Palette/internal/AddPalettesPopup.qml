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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Palette 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledPopupView {
    id: root

    property PaletteProvider paletteProvider: null
    property alias model: palettesList.model
    property int maxHeight: 400

    property int popupAvailableWidth: 0

    contentWidth: contentColumn.width
    contentHeight: contentColumn.height

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "AddPalettesPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    onOpened: {
        createCustomPaletteButton.navigation.requestActive()
    }

    signal addCustomPaletteRequested()

    Column {
        id: contentColumn

        width: root.popupAvailableWidth - 2 * root.margins
        height: childrenRect.height

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

            objectName: "CreateCustomPalette"
            navigation.panel: root.navigationPanel
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

        StyledListView {
            id: palettesList
            height: Math.min(availableHeight, contentHeight)
            width: parent.width + root.margins

            readonly property int availableHeight:
                root.maxHeight - header.height - createCustomPaletteButton.height - 2 * contentColumn.spacing

            scrollBarThickness: 6

            spacing: 8
            visible: count > 0

            delegate: Item {
                id: morePalettesDelegate

                width: parent.width - root.margins
                height: addButton.height

                property bool added: false
                property bool removed: false

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

                        objectName: "Add"+model.display+"Palette"
                        navigation.panel: root.navigationPanel
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
