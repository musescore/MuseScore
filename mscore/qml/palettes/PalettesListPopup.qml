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
import MuseScore.Utils 3.3

StyledPopup {
    id: palettesListPopup
    property PaletteWorkspace paletteWorkspace: null

    property bool inMenuAction: false

    signal addCustomPaletteRequested()

    Text {
        id: header
        anchors.top: parent.top
        text: qsTr("More palettes")
        font: globalStyle.font
        color: globalStyle.windowText
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
                model = paletteWorkspace.availableExtraPalettesModel();
            }
        }

        delegate: ItemDelegate { // TODO: use ItemDelegate or just Item?
            id: morePalettesDelegate
            width: parent.width
            height: addButton.height
            topPadding: 0
            bottomPadding: 0

            property bool added: false // TODO: store in some model
            property bool removed: false

            text: model.display

            contentItem: Item {
                height: parent.availableHeight
                width: parent.availableWidth
                Item {
                    visible: !(morePalettesDelegate.added || morePalettesDelegate.removed)
                    anchors.fill: parent

                    Text {
                        height: parent.height
                        anchors.left: parent.left
                        anchors.right: addButton.left
                        text: morePalettesDelegate.text
                        font: globalStyle.font
                        color: globalStyle.windowText
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHLeft
                        elide: Text.ElideRight

                        MouseArea {
                            id: rightClickArea
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton

                            onClicked: {
                                if (!model.custom)
                                    return;
                                contextMenu.paletteIndex = model.paletteIndex;
                                contextMenu.item = morePalettesDelegate;
                                contextMenu.popup();
                            }
                        }
                    }
                    StyledButton {
                        id: addButton
//                         height: parent.height
                        anchors.right: parent.right
                        text: qsTr("Add")
                        ToolTip.visible: hovered
                        ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                        ToolTip.text: qsTr("Add %1 palette").arg(model.display)
                        Accessible.description: ToolTip.text

                        onClicked: {
                            if (paletteWorkspace.addPalette(model.paletteIndex))
                                morePalettesDelegate.added = true; // TODO: store in some model
                        }
                    }
                }

                Text {
                    visible: morePalettesDelegate.added || morePalettesDelegate.removed
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: morePalettesDelegate.added ? qsTr("%1 Added!").arg(model.display) : (morePalettesDelegate.removed ? qsTr("%1 removed").arg(model.display) : "")
                    font: globalStyle.font
                    color: globalStyle.windowText
                    elide: Text.ElideMiddle
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

    StyledButton {
        id: createCustomPaletteButton
        anchors.bottom: parent.bottom
        width: parent.width
        text: qsTr("Create custom palette")
        onClicked: {
            addCustomPaletteRequested();
            palettesListPopup.close();
        }
    }

    Menu {
        id: contextMenu
        property var paletteIndex: null
        property ItemDelegate item: null

        MenuItem {
            text: qsTr("Delete")
            onTriggered: {
                palettesListPopup.inMenuAction = true;
                if (paletteWorkspace.removeCustomPalette(contextMenu.paletteIndex))
                    contextMenu.item.removed = true;
                palettesListPopup.inMenuAction = false;
            }
        }
    }
}
