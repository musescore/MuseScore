/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Palette 1.0

Item {
    id: root

    property PaletteProvider paletteProvider: null

    property bool isSearchOpened: false
    readonly property bool isSearchFieldFocused: searchField.activeFocus
    readonly property string searchText: searchField.searchText

    property alias popupMaxHeight: addPalettesPopup.maxHeight
    property var popupAnchorItem: null

    property alias navigation: navPanel

    signal addCustomPaletteRequested(var paletteName)

    implicitHeight: childrenRect.height

    function startSearch() {
        isSearchOpened = true
        searchField.forceActiveFocus()
        searchField.selectAll()
    }

    function endSearch() {
        isSearchOpened = false
        addPalettesButton.forceActiveFocus()
    }

    NavigationPanel {
        id: navPanel
        name: "PalettesHeader"
        enabled: root.enabled && root.visible
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    QtObject {
        id: prv

        property var openedPopup: null
        property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(popup, model) {
            if (isPopupOpened) {
                openedPopup.close()
                openedPopup = null
            }

            if (Boolean(popup)) {
                openedPopup = popup
                popup.model = model
                popup.open()
            }
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                openedPopup.close()
                openedPopup = null
            }
        }
    }

    FlatButton {
        id: addPalettesButton
        objectName: "AddPalettesBtn"

        anchors.left: parent.left
        anchors.right: startSearchButton.left
        anchors.rightMargin: 8

        navigation.panel: navPanel
        navigation.order: 1

        text: qsTrc("palette", "Add Palettes")
        visible: !root.isSearchOpened
        enabled: visible

        onClicked: {
            if (prv.openedPopup == addPalettesPopup) {
                prv.closeOpenedPopup()
                return
            }

            prv.openPopup(addPalettesPopup, paletteProvider.availableExtraPalettesModel())
            createCustomPalettePopup.visible = false
        }

        AddPalettesPopup {
            id: addPalettesPopup
            paletteProvider: root.paletteProvider

            navigationParentControl: addPalettesButton.navigation

            popupAvailableWidth: root ? root.width : 0
            anchorItem: root.popupAnchorItem

            onAddCustomPaletteRequested: {
                createCustomPalettePopup.open()
            }
        }
    }

    FlatButton {
        id: startSearchButton
        objectName: "SearchPalettesBtn"
        anchors.right: parent.right

        navigation.panel: navPanel
        navigation.order: 2

        toolTipTitle: qsTrc("palette", "Search Palettes")

        icon: IconCode.SEARCH
        visible: !root.isSearchOpened
        enabled: visible

        onClicked: {
            prv.closeOpenedPopup()
            createCustomPalettePopup.visible = false
            root.startSearch()
        }
    }

    SearchField {
        id: searchField
        objectName: "SearchPalettesField"
        width: parent.width

        navigation.panel: navPanel
        navigation.order: 3
        navigation.onActiveChanged: {
            if (navigation.active) {
                searchField.forceActiveFocus()
            }
        }

        clearTextButtonVisible: true
        clearTextButton.objectName: "SearchPalettesFieldClose"
        clearTextButton.navigation.order: 4
        clearTextButton.navigation.onTriggered: {
            root.endSearch()
            addPalettesButton.navigation.requestActive()
        }

        onTextCleared: {
            root.endSearch()
        }

        visible: root.isSearchOpened
        onVisibleChanged: {
            if (!searchField.visible) {
                addPalettesButton.navigation.requestActive()
            }
        }

        onSearchTextChanged: resultsTimer.restart()
        onActiveFocusChanged: {
            resultsTimer.stop();
            Accessible.name = qsTrc("palette", "Palette Search")
        }

        Timer {
            id: resultsTimer
            interval: 500
            onTriggered: {
                parent.Accessible.name = parent.searchText.length === 0
                        ? qsTrc("palette", "Palette Search")
                        : qsTrc("palette", "%n palette(s) match(es)", "", paletteTree.count);
            }
        }

        Keys.onEscapePressed: root.endSearch()
    }

    CreateCustomPalettePopup {
        id: createCustomPalettePopup

        anchorItem: addPalettesButton
        width: parent.width

        onAddCustomPaletteRequested: function(paletteName) {
            root.addCustomPaletteRequested(paletteName)
        }
    }
}
