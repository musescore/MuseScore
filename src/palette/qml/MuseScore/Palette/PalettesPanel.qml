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

import "internal"

Rectangle {
    id: root

    property NavigationSection navigationSection: null

    readonly property PaletteProvider paletteProvider: paletteRootModel.paletteProvider

    implicitHeight: 4 * palettesPanelHeader.implicitHeight
    implicitWidth: paletteTree.implicitWidth

    enabled: paletteRootModel.paletteEnabled

    function applyCurrentPaletteElement() {
        paletteTree.applyCurrentElement();
    }

    color: ui.theme.backgroundPrimaryColor

    PaletteRootModel {
        id: paletteRootModel

        onPaletteSearchRequested: {
            palettesPanelHeader.startSearch()
        }
    }

    PalettesPanelHeader {
        id: palettesPanelHeader

        paletteProvider: root.paletteProvider

        popupMaxHeight: root.height - palettesPanelHeader.height

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12

        navigation.section: root.navigationSection
        navigation.enabled: root.visible
        navigation.order: 2

        onAddCustomPaletteRequested: paletteTree.insertCustomPalette(0, paletteName);
    }

    StyledTextLabel {
        id: searchHint

        anchors.top: palettesPanelHeader.bottom
        anchors.topMargin: 26
        anchors.horizontalCenter: parent.horizontalCenter

        text: qsTrc("palette", "Start typing to search all palettes")

        visible: palettesPanelHeader.isSearchOpened && !Boolean(palettesPanelHeader.searchText)
    }

    PaletteTree {
        id: paletteTree
        clip: true
        paletteProvider: root.paletteProvider
        backgroundColor: root.color

        navigation.section: root.navigationSection
        navigation.enabled: root.visible
        navigation.order: 5

        filter: palettesPanelHeader.searchText
        enableAnimations: !palettesPanelHeader.isSearchFieldFocused
        searchOpened: palettesPanelHeader.isSearchOpened

        anchors {
            top: palettesPanelHeader.bottom
            topMargin: 3
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        visible: !searchHint.visible
    }

    Rectangle {
        // Shadow overlay for Tours. The usual overlay doesn't cover palettes
        // as they reside in a window container above the main MuseScore window.
        visible: paletteRootModel.needShowShadowOverlay
        anchors.fill: parent
        z: 1000

        color: ui.theme.strokeColor
        opacity: 0.5
    }
}
