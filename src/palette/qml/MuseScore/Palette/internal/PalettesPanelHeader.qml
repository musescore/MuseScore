/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.Palette

Item {
    id: root

    property PaletteProvider paletteProvider: null

    property bool isSearchOpened: false
    readonly property bool isSearchFieldFocused: searchField.activeFocus
    readonly property string searchText: searchField.searchText

    property int popupMaxHeight: 400
    property var popupAnchorItem: null

    property alias navigation: navPanel

    signal applyCurrentPaletteElementRequested()
    signal addCustomPaletteRequested(var paletteName)

    implicitHeight: childrenRect.height

    function startSearch() {
        isSearchOpened = true
        Qt.callLater(searchField.forceActiveFocus)
        Qt.callLater(searchField.selectAll)
    }

    function endSearch() {
        isSearchOpened = false
        Qt.callLater(addPalettesButton.forceActiveFocus)
        Qt.callLater(addPalettesButton.navigation.requestActive)
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

    PopupButton {
        id: addPalettesButton
        objectName: "AddPalettesBtn"

        anchors.left: parent.left
        anchors.right: startSearchButton.left
        anchors.rightMargin: 8

        navigation.panel: navPanel
        navigation.order: 1

        text: qsTrc("palette", "Add palettes")
        visible: !root.isSearchOpened
        enabled: visible

        popupAnchorItem: root.popupAnchorItem

        popupComponent: AddPalettesPopup {
            paletteProvider: root.paletteProvider
            model: root.paletteProvider ? root.paletteProvider.availableExtraPalettesModel() : null
            popupAvailableWidth: root.width
            maxHeight: root.popupMaxHeight

            onAddCustomPaletteRequested: {
                addPalettesButton.close()
                createCustomPalettePopupLoader.toggleOpened()
            }
        }

        StyledPopupLoader {
            id: createCustomPalettePopupLoader

            popupAnchorItem: root.popupAnchorItem

            sourceComponent: CreateCustomPalettePopup {
                popupAvailableWidth: root.width

                onAddCustomPaletteRequested: function(paletteName) {
                    root.addCustomPaletteRequested(paletteName)
                }
            }
        }
    }

    FlatButton {
        id: startSearchButton
        objectName: "SearchPalettesBtn"
        anchors.right: parent.right

        navigation.panel: navPanel
        navigation.order: 2

        toolTipTitle: qsTrc("palette", "Search palettes")

        icon: IconCode.SEARCH
        visible: !root.isSearchOpened
        enabled: visible

        onClicked: {
            addPalettesButton.close()
            createCustomPalettePopupLoader.close()
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

        onTextCleared: {
            root.endSearch()
        }

        visible: root.isSearchOpened

        Keys.onEscapePressed: root.endSearch()

        onAccepted: root.applyCurrentPaletteElementRequested()
    }
}
