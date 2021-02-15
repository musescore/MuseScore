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

import "utils.js" as Utils

Item {
    id: header

    property PaletteWorkspace paletteWorkspace: null
    property string searchText: searchTextInput.searchText
    readonly property bool searching: searchTextInput.activeFocus
    property bool searchOpened: searchTextInput.visible

    property alias popupMaxHeight: palettePopup.maxHeight

    signal addCustomPaletteRequested(var paletteName)

    implicitHeight: childrenRect.height

    function searchSelectAll() {
        searchTextInput.forceActiveFocus()
        searchTextInput.selectAll()
    }

    function toggleSearch() {
        searchTextButton.visible = !searchTextButton.visible
        searchTextInput.visible = !searchTextInput.visible
        morePalettesButton.visible = !searchTextInput.visible

        if (searchTextInput.visible) {
            searchTextInput.forceActiveFocus()
        }
    }

    FlatButton {
        id: morePalettesButton
        anchors.left: parent.left
        anchors.right: searchTextButton.left
        anchors.rightMargin: 8
        text: qsTrc("palette", "Add Palettes")
        onClicked: palettePopup.visible = !palettePopup.visible
    }

    FlatButton {
        id: searchTextButton
        anchors.right: parent.right
        icon: IconCode.SEARCH

        onClicked: {
            toggleSearch()
        }
    }

    SearchField {
        id: searchTextInput
        width: parent.width

        visible: false

        onSearchTextChanged: resultsTimer.restart()
        onActiveFocusChanged: {
            resultsTimer.stop();
            Accessible.name = qsTrc("palette", "Palette Search")
        }

        Timer {
            id: resultsTimer
            interval: 500
            onTriggered: {
                parent.Accessible.name = parent.searchText.length === 0 ? qsTrc("palette", "Palette Search") : qsTrc("palette", "%n palette(s) match(es)", "", paletteTree.count);
            }
        }

        KeyNavigation.tab: paletteTree.currentTreeItem

        Keys.onDownPressed: paletteTree.focusFirstItem();
        Keys.onUpPressed: paletteTree.focusLastItem();
        Keys.onEscapePressed: toggleSearch()

        clearTextButtonVisible: true
        onTextCleared: {
            toggleSearch()
        }
    }

    CreateCustomPalettePopup {
        id: createCustomPalettePopup

        y: morePalettesButton.y + morePalettesButton.height
        arrowX: morePalettesButton.x + morePalettesButton.width / 2

        onAddCustomPaletteRequested: {
            header.addCustomPaletteRequested(paletteName)
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

        onAddCustomPaletteRequested: {
            createCustomPalettePopup.open()
        }
    }

    Connections {
        target: palettesWidget
        function onHasFocusChanged() {
            if (!palettesWidget.hasFocus && !palettePopup.inMenuAction)
                palettePopup.visible = false;
        }
    }
}
