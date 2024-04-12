/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
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

    QtObject {
        id: prv

        property var openedPopup: null
        property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(popup, model) {
            if (isPopupOpened) {
                if (openedPopup === popup) {
                    resetOpenedPopup()
                    return
                }

                resetOpenedPopup()
            }

            if (Boolean(popup)) {
                openedPopup = popup

                if (Boolean(model)) {
                    popup.model = model
                }

                popup.open()
            }
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                resetOpenedPopup()
            }
        }

        function resetOpenedPopup() {
            openedPopup.close()
            openedPopup = null
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

        text: qsTrc("palette", "Add palettes")
        visible: !root.isSearchOpened
        enabled: visible

        onClicked: {
            prv.openPopup(addPalettesPopup, paletteProvider.availableExtraPalettesModel())
        }

        AddPalettesPopup {
            id: addPalettesPopup
            paletteProvider: root.paletteProvider

            popupAvailableWidth: root ? root.width : 0
            anchorItem: root.popupAnchorItem

            onAddCustomPaletteRequested: {
                prv.openPopup(createCustomPalettePopup)
            }
        }

        CreateCustomPalettePopup {
            id: createCustomPalettePopup

            popupAvailableWidth: root ? root.width : 0
            anchorItem: root.popupAnchorItem

            onAddCustomPaletteRequested: function(paletteName) {
                root.addCustomPaletteRequested(paletteName)
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
            prv.closeOpenedPopup()
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

        onAccepted: applyCurrentPaletteElement()
    }
}
