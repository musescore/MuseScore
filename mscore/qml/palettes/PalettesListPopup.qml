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
import MuseScore.Palette 3.3

StyledPopup {
    id: palettesListPopup
    property PaletteWorkspace paletteWorkspace: null

    Text {
        id: header
        anchors.top: parent.top
        text: qsTr("More palettes")
    }

    ToolSeparator {
        id: topSeparator
        orientation: Qt.Horizontal
        anchors.top: header.bottom
        width: parent.width
    }

    ListView {
        anchors {
            top: topSeparator.bottom
            bottom: bottomSeparator.top
            left: parent.left
            right: parent.right
        }
        clip: true
        model: null

        ScrollBar.vertical: ScrollBar {}

        spacing: 8

        onVisibleChanged: {
            if (visible) {
                model = paletteWorkspace.availableExtraPalettePanelsModel();
            }
        }

        delegate: ItemDelegate { // TODO: use ItemDelegate or just Item?
            id: morePalettesDelegate
            width: parent.width
            height: addButton.height
            topPadding: 0
            bottomPadding: 0
            property bool added: false // TODO: store in some model

            text: model.display

            contentItem: Item {
                height: parent.availableHeight
                width: parent.availableWidth
                Item {
                    visible: !morePalettesDelegate.added
                    anchors.fill: parent

                    Text {
                        height: parent.height
                        text: morePalettesDelegate.text
                        verticalAlignment: Text.AlignVCenter
                    }
                    Button {
                        id: addButton
//                         height: parent.height
                        anchors.right: parent.right
                        text: qsTr("Add")
                        ToolTip.visible: hovered
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                        ToolTip.text: qsTr("Add %1 palette").arg(model.display)
                        Accessible.description: ToolTip.text

                        onClicked: {
                            paletteWorkspace.addPalette(model.display);
                            morePalettesDelegate.added = true; // TODO: store in some model
                        }
                    }
                }

                Text {
                    visible: morePalettesDelegate.added
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("%1 Added!").arg(model.display)
                }
            }
        }
    }

    ToolSeparator {
        id: bottomSeparator
        orientation: Qt.Horizontal
        anchors.bottom: createCustomPaletteButton.top
        width: parent.width
    }

    Button {
        id: createCustomPaletteButton
        anchors.bottom: parent.bottom
        width: parent.width
        text: qsTr("Create custom palette")
        onClicked: {
            paletteWorkspace.addCustomPalette();
            palettesListPopup.close();
        }
    }
}
