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

import "utils.js" as Utils

Item {
    id: header

    property PaletteWorkspace paletteWorkspace: null
    property string cellFilter: searchTextInput.text
    readonly property bool searching: searchTextInput.activeFocus || searchTextClearButton.activeFocus

    signal addCustomPaletteRequested()

    implicitHeight: childrenRect.height

    RowLayout {
        width: parent.width

        height: searchTextInput.height
        spacing: 8

        TextField {
            id: searchTextInput
            Layout.fillWidth: true

            placeholderText: qsTr("Search")
            font: globalStyle.font

            onTextChanged: resultsTimer.restart()
            onActiveFocusChanged: {
                resultsTimer.stop();
                Accessible.name = qsTr("Palette Search")
            }

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

            KeyNavigation.tab: paletteTree.currentTreeItem

            Keys.onDownPressed: paletteTree.focusFirstItem();
            Keys.onUpPressed: paletteTree.focusLastItem();

            StyledToolButton {
                id: searchTextClearButton
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                    margins: 1
                }
                width: height
                visible: searchTextInput.text.length && searchTextInput.width > 2 * width
                flat: true
                onClicked: searchTextInput.clear()
                activeFocusOnTab: false // don't annoy keyboard users tabbing to palette (they can use Ctrl+A, Delete to clear search)

                padding: 4

                text: qsTr("Clear search text")

                onHoveredChanged: {
                    if (hovered) {
                        mscore.tooltip.item = searchTextClearButton;
                        mscore.tooltip.text = searchTextClearButton.text;
                    } else if (mscore.tooltip.item == searchTextClearButton) {
                        mscore.tooltip.item = null;
                    }
                }

                contentItem: StyledIcon {
                    source: "icons/clear.png"
                }
            }
        }

        StyledButton {
            id: morePalettesButton
            Layout.preferredHeight: searchTextInput.height
            Layout.preferredWidth: searchTextInput.height
            text: qsTr("Add Palettes")
            contentItem: StyledIcon {
                source: "icons/plus.png"
            }
            onClicked: {
                palettesListPopup.visible = !palettesListPopup.visible
                paletteOptionsPopup.visible = false
            }
        }

        StyledButton {
            id: paletteOptionsButton
            Layout.preferredHeight: searchTextInput.height
            Layout.preferredWidth: searchTextInput.height
            text: qsTr("Options")
            contentItem: StyledIcon {
                source: "icons/menu_dots.svg"
            }
            onClicked: {
                paletteOptionsPopup.visible = !paletteOptionsPopup.visible
                palettesListPopup.visible = false
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

    PaletteOptionsPopup {
        id: paletteOptionsPopup
        paletteWorkspace: palettesWidget.paletteWorkspace

        visible: false

        arrowAnchorItem: paletteOptionsButton
        y: paletteOptionsButton.y + paletteOptionsButton.height + Utils.style.popupMargin
        width: parent.width

        modal: false
        dim: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside | Popup.CloseOnPressOutsideParent
    }

    Connections {
        target: palettesWidget
        function onHasFocusChanged() {
            if (!palettesWidget.hasFocus) {
                if (!palettesListPopup.inMenuAction)
                    palettesListPopup.visible = false;
                if (!paletteOptionsPopup.inMenuAction)
                    paletteOptionsPopup.visible = false;
            }
        }
    }

    Connections {
        target: mscore
        function onPaletteSearchRequested() {
            searchTextInput.forceActiveFocus()
            searchTextInput.selectAll()
        }
    }
}
