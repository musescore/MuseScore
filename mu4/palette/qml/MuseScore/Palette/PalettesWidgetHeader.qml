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

import "utils.js" as Utils

Item {
    id: header

    property PaletteWorkspace paletteWorkspace: null
    property string cellFilter: searchTextInput.text
    readonly property bool searching: searchTextInput.activeFocus || searchTextClearButton.activeFocus

    property alias popupMaxHeight: palettePopup.maxHeight

    signal addCustomPaletteRequested()

    implicitHeight: childrenRect.height

    function searchSelectAll() {
        searchTextInput.forceActiveFocus()
        searchTextInput.selectAll()
    }

    StyledButton {
        id: morePalettesButton
        height: searchTextInput.height
        width: parent.width / 2 - 4
        text: qsTr("Add Palettes")
        onClicked: palettePopup.visible = !palettePopup.visible
    }

    TextField {
        id: searchTextInput
        width: parent.width / 2 - 4
        anchors.right: parent.right

        placeholderText: qsTr("Search")
        font: ui.theme.font

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

        color: ui.theme.fontPrimaryColor

        background: Rectangle {
            color: ui.theme.backgroundPrimaryColor
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
                    ui.tooltip.show(searchTextClearButton, searchTextClearButton.text)
                } else {
                    ui.tooltip.hide(searchTextClearButton)
                }
            }

            contentItem: StyledIcon {
                source: "icons/clear.png"
            }
        }
    }

    PalettesListPopup {
        id: palettePopup
        paletteWorkspace: header.paletteWorkspace

        visible: false

        y: morePalettesButton.y + morePalettesButton.height + Utils.style.popupMargin
        width: parent.width

        arrowX: morePalettesButton.x + morePalettesButton.width / 2 - x

        modal: true
        dim: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onAddCustomPaletteRequested: header.addCustomPaletteRequested()
    }

    Connections {
        target: palettesWidget
        function onHasFocusChanged() {
            if (!palettesWidget.hasFocus && !palettePopup.inMenuAction)
                palettePopup.visible = false;
        }
    }
}
