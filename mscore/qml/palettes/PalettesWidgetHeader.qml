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
import QtQuick.Layouts 1.1

import MuseScore.Palette 3.3
import MuseScore.Utils 3.3

import "utils.js" as Utils

Item {
    id: header

    property PaletteWorkspace paletteWorkspace: null
    property bool searchTextFieldShown: false
    readonly property bool searching: searchTextField.activeFocus || stopSearchButton.activeFocus
    property string cellFilter: searchTextField.text

    signal addCustomPaletteRequested()

    implicitHeight: childrenRect.height

    function paletteSearchRequested() {
        searchTextFieldShown = true
        searchTextField.forceActiveFocus()
        searchTextField.selectAll()
    }

    function showPaletteOptionsMenu() {
        paletteOptionsMenu.x = paletteOptionsButton.x + paletteOptionsButton.width - paletteOptionsMenu.width;
        paletteOptionsMenu.y = paletteOptionsButton.y;
        collapseAllMenuItem.enabled = paletteTree.numberOfExpandedPalettes() > 0
        expandAllMenuItem.enabled = paletteTree.numberOfCollapsedPalettes() > 0 && !paletteWorkspace.singlePalette
        paletteOptionsMenu.open();
    }

    RowLayout {
        width: parent.width

        height: searchTextField.height
        spacing: 6

        StyledButton {
            id: morePalettesButton
            visible: !searchTextFieldShown
            Layout.preferredHeight: searchTextField.height
            Layout.fillWidth: true
            text: qsTr("Add Palettes")
            onClicked: {
                palettesListPopup.visible = !palettesListPopup.visible
            }
        }

        StyledButton {
            id: startSearchButton
            visible: !searchTextFieldShown
            Layout.preferredHeight: searchTextField.height
            Layout.preferredWidth: searchTextField.height
            text: qsTr("Search")
            padding: 5
            contentItem: StyledIcon {
                source: "icons/MagnifyingGlass.svg"
            }
            onClicked: {
                palettesListPopup.visible = false
                paletteSearchRequested()
            }
        }

        StyledButton {
            id: paletteOptionsButton
            visible: !searchTextFieldShown
            Layout.preferredHeight: searchTextField.height
            Layout.preferredWidth: searchTextField.height
            padding: 4
            text: qsTr("Options")
            contentItem: StyledIcon {
                source: "icons/ThreeDotMenu.svg"
            }
            onClicked: {
                palettesListPopup.visible = false
                showPaletteOptionsMenu()
            }
        }

        TextField {
            id: searchTextField
            visible: searchTextFieldShown
            Layout.fillWidth: true

            placeholderText: qsTr("Search")
            font: globalStyle.font

            onTextChanged: resultsTimer.restart()
            onActiveFocusChanged: {
                resultsTimer.stop();
                Accessible.name = qsTr("Palette Search")
            }

            selectByMouse: true

            Timer {
                id: resultsTimer
                interval: 500
                onTriggered: {
                    parent.Accessible.name = parent.text.length === 0 ? qsTr("Palette Search") : qsTr("%n palette(s) match(es)", "", paletteTree.count);
                }
            }

            color: globalStyle.text

            background: Rectangle {
                color: globalStyle.base
                border.color: "#aeaeae"
            }

            Keys.onDownPressed: paletteTree.focusFirstItem();
            Keys.onUpPressed: paletteTree.focusLastItem();

            StyledToolButton {
                id: stopSearchButton
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                    margins: 1
                }
                width: height
                flat: true
                onClicked: {
                    searchTextField.clear()
                    searchTextFieldShown = false
                    paletteTree.currentTreeItem.forceActiveFocus()
                }

                padding: 5

                text: qsTr("Stop search")

                onHoveredChanged: {
                    if (hovered) {
                        mscore.tooltip.item = stopSearchButton;
                        mscore.tooltip.text = stopSearchButton.text;
                    } else if (mscore.tooltip.item === stopSearchButton) {
                        mscore.tooltip.item = null;
                    }
                }

                contentItem: StyledIcon {
                    source: "icons/CloseButton.svg"
                }
            }
        }
    }

    PalettesListPopup {
        id: palettesListPopup
        paletteWorkspace: palettesWidget.paletteWorkspace

        visible: false

        arrowAnchorItem: morePalettesButton
        y: morePalettesButton.y + morePalettesButton.height + Utils.style.popupMargin
        width: parent.width
        maxHeight: paletteTree.height * 0.8

        modal: false
        dim: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside | Popup.CloseOnPressOutsideParent

        onAddCustomPaletteRequested: header.addCustomPaletteRequested()
    }

    Menu {
        id: paletteOptionsMenu

        MenuItem {
            text: qsTr("Open only one Palette at a time")
            checkable: true
            checked: paletteWorkspace.singlePalette
            onTriggered: paletteWorkspace.singlePalette = checked
        }

        MenuSeparator {}

        MenuItem {
            id: collapseAllMenuItem
            text: qsTr("Collapse all Palettes")
            onTriggered: paletteTree.expandCollapseAll(false)
        }

        MenuItem {
            id: expandAllMenuItem
            text: qsTr("Expand all Palettes")
            onTriggered: paletteTree.expandCollapseAll(true)
        }
    }

    Connections {
        target: palettesWidget
        onHasFocusChanged: {
            if (!palettesWidget.hasFocus && !palettesListPopup.inMenuAction)
                palettesListPopup.visible = false;
        }
    }
}
