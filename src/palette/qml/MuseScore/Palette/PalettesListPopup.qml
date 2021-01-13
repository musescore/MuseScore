//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.12

import MuseScore.Palette 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

StyledPopup {
    id: palettesListPopup

    property PaletteWorkspace paletteWorkspace: null
    property int maxHeight: 400

    height: column.implicitHeight + topPadding + bottomPadding
    width: parent.width

    property bool inMenuAction: false

    signal addCustomPaletteRequested()

    Column {
        id: column

        width: parent.width

        spacing: 12

        StyledTextLabel {
            id: header
            text: qsTrc("palette", "More palettes")
        }

        FlatButton {
            id: createCustomPaletteButton
            width: parent.width
            text: qsTrc("palette", "Create custom palette")
            onClicked: {
                addCustomPaletteRequested();
                palettesListPopup.close();
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: !palettesList.count
            text: qsTrc("palette", "All palettes were added")
            wrapMode: Text.WordWrap
        }

        ListView {
            id: palettesList
            width: parent.width
            clip: true
            property var extraPalettesModel: null // keeping a separate variable for a model prevents it from being deleted by QML
            model: extraPalettesModel

            readonly property int availableHeight: palettesListPopup.maxHeight - header.height - createCustomPaletteButton.height
            height: Math.min(availableHeight, contentHeight)

            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: StyledScrollBar {}

            spacing: 8

            onVisibleChanged: {
                if (visible) {
                    extraPalettesModel = paletteWorkspace.availableExtraPalettesModel();
                }
            }

            delegate: Item {
                id: morePalettesDelegate

                width: parent.width
                height: addButton.height

                property bool added: false
                property bool removed: false

                Accessible.name: model.display

                RowLayout {
                    visible: !(morePalettesDelegate.added || morePalettesDelegate.removed)
                    anchors.fill: parent

                    spacing: 8

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

                        ToolTip.text: qsTrc("palette", "Add %1 palette").arg(model.display)
                        Accessible.description: ToolTip.text

                        onHoveredChanged: {
                            if (hovered) {
                                ui.tooltip.show(addButton, addButton.ToolTip.text)
                            } else {
                                ui.tooltip.hide(addButton)
                            }
                        }

                        onClicked: {
                            if (paletteWorkspace.addPalette(model.paletteIndex)) {
                                morePalettesDelegate.added = true
                            }
                        }
                    }
                }

                StyledTextLabel {
                    anchors.fill: parent

                    visible: morePalettesDelegate.added || morePalettesDelegate.removed
                    text: morePalettesDelegate.added ? qsTrc("palette", "%1 added").arg(model.display) : (morePalettesDelegate.removed ? qsTrc("palette", "%1 removed").arg(model.display) : "")
                    elide: Text.ElideMiddle
                    font: ui.theme.bodyBoldFont
                }
            }
        }
    }
}
