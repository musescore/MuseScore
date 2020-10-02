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
            text: qsTr("More palettes")
        }

        FlatButton {
            id: createCustomPaletteButton
            width: parent.width
            text: qsTr("Create custom palette")
            onClicked: {
                addCustomPaletteRequested();
                palettesListPopup.close();
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: !palettesList.count
            text: qsTr("All palettes were added")
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

            ScrollBar.vertical: ScrollBar {}

            spacing: 8

            onVisibleChanged: {
                if (visible) {
                    extraPalettesModel = paletteWorkspace.availableExtraPalettesModel();
                }
            }

            delegate: ItemDelegate { // TODO: use ItemDelegate or just Item?
                id: morePalettesDelegate
                width: parent.width
                height: addButton.height
                topPadding: 0
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0

                background: Item {}

                property bool added: false // TODO: store in some model
                property bool removed: false

                text: model.display

                contentItem: Item {
                    height: parent.availableHeight
                    width: parent.availableWidth
                    Item {
                        visible: !(morePalettesDelegate.added || morePalettesDelegate.removed)
                        anchors.fill: parent

                        StyledTextLabel {
                            height: parent.height
                            anchors.left: parent.left
                            anchors.right: addButton.left
                            text: morePalettesDelegate.text
                            horizontalAlignment: Text.AlignHLeft
                        }

                        FlatButton {
                            id: addButton
                            anchors.right: parent.right
                            icon: IconCode.PLUS

                            ToolTip.text: qsTr("Add %1 palette").arg(model.display)
                            Accessible.description: ToolTip.text

                            onHoveredChanged: {
                                if (hovered) {
                                    ui.tooltip.show(addButton, addButton.ToolTip.text)
                                } else {
                                    ui.tooltip.hide(addButton)
                                }
                            }

                            onClicked: {
                                if (paletteWorkspace.addPalette(model.paletteIndex))
                                    morePalettesDelegate.added = true; // TODO: store in some model
                            }
                        }
                    }

                    StyledTextLabel {
                        visible: morePalettesDelegate.added || morePalettesDelegate.removed
                        anchors.fill: parent
                        text: morePalettesDelegate.added ? qsTr("%1 added").arg(model.display) : (morePalettesDelegate.removed ? qsTr("%1 removed").arg(model.display) : "")
                        elide: Text.ElideMiddle
                        font.bold: true
                    }
                }
            }
        }
    }
}
