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
import QtQuick.Layouts 1.15

import MuseScore.Shortcuts 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Palette 1.0

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property alias contextMenuModel: contextMenuModel

    readonly property PaletteProvider paletteProvider: paletteRootModel.paletteProvider

    implicitHeight: 4 * palettesPanelHeader.implicitHeight
    implicitWidth: paletteTree.implicitWidth

    enabled: paletteRootModel.paletteEnabled

    function applyCurrentPaletteElement() {
        paletteTree.applyCurrentElement();
    }

    PalettesPanelContextMenuModel {
        id: contextMenuModel

        onExpandCollapseAllRequested: function(expand) {
            paletteTree.expandCollapseAll(expand)
        }
    }

    Component.onCompleted: {
        contextMenuModel.load()
        console.log("Loading model from panel")
        shortcutsModel.load()
    }

    PaletteRootModel {
        id: paletteRootModel

        onPaletteSearchRequested: {
            palettesPanelHeader.startSearch()
        }
    }

    ColumnLayout {
        id: contentColumn

        readonly property int sideMargin: 12

        anchors.fill: parent
        anchors.leftMargin: sideMargin
        anchors.rightMargin: sideMargin

        spacing: sideMargin

        PalettesPanelHeader {
            id: palettesPanelHeader

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            paletteProvider: root.paletteProvider

            popupMaxHeight: contentColumn.height - palettesPanelHeader.height
            popupAnchorItem: root

            navigation.section: root.navigationSection
            navigation.order: 2

            onAddCustomPaletteRequested: function(paletteName) {
                paletteTree.insertCustomPalette(0, paletteName)
            }
        }

        StyledTextLabel {
            id: searchHint

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            text: qsTrc("palette", "Start typing to search all palettes")
            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap

            visible: palettesPanelHeader.isSearchOpened && !Boolean(palettesPanelHeader.searchText)
        }

        StyledTextLabel {
            id: notFoundHint

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            text: qsTrc("palette", "No results found")
            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap

            visible: !paletteTree.isResultFound
        }

        ShortcutsModel {
            id: shortcutsModel
        }

        PaletteTree {
            id: paletteTree

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: -contentColumn.sideMargin
            Layout.rightMargin: -contentColumn.sideMargin

            clip: true
            paletteProvider: root.paletteProvider

            navigation.section: root.navigationSection
            navigation.order: 5

            filter: palettesPanelHeader.searchText
            enableAnimations: !palettesPanelHeader.isSearchFieldFocused
            searchOpened: palettesPanelHeader.isSearchOpened

            visible: !searchHint.visible
        }
    }
}
