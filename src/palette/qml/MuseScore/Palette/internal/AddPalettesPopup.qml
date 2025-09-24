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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Palette

StyledPopupView {
    id: root

    property PaletteProvider paletteProvider: null
    property alias model: palettesList.model
    property int maxHeight: 400

    property int popupAvailableWidth: 0

    contentWidth: popupAvailableWidth - 2 * margins
    contentHeight: Math.min(maxHeight - 2 * margins - 2 * padding - 1, 
                            contentColumn.implicitHeight)

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

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent
        spacing: 12

        StyledTextLabel {
            id: header
            Layout.fillWidth: true
            text: qsTrc("palette", "More palettes")
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        FlatButton {
            id: createCustomPaletteButton
            Layout.fillWidth: true
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: palettesList.count <= 0
            text: qsTrc("palette", "All palettes were added")
            wrapMode: Text.WordWrap
        }

        StyledListView {
            id: palettesList

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: -root.margins
            Layout.rightMargin: -root.margins

            implicitHeight: contentHeight

            scrollBarThickness: 6

            spacing: 8
            visible: count > 0

            delegate: Item {
                id: morePalettesDelegate

                width: ListView.view.width
                implicitWidth: rowLayout.implicitWidth
                implicitHeight: rowLayout.implicitHeight

                property bool added: false
                property bool removed: false

                RowLayout {
                    id: rowLayout
                    anchors.fill: parent
                    anchors.leftMargin: root.margins
                    anchors.rightMargin: root.margins
                    spacing: 8
                    visible: !(morePalettesDelegate.added || morePalettesDelegate.removed)

                    StyledTextLabel {
                        Layout.fillWidth: true

                        text: model.display
                        horizontalAlignment: Text.AlignHLeft
                    }

                    FlatButton {
                        id: addButton

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
