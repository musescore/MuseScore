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

import QtQuick 2.15
import QtQuick.Controls 2.1
import QtQuick.Window 2.2
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Palette 1.0

import "internal"

Rectangle {

    id: palettesWidget

    property KeyNavigationSection keynavSection: null

    readonly property PaletteWorkspace paletteWorkspace: paletteRootModel.paletteWorkspace

    implicitHeight: 4 * palettesWidgetHeader.implicitHeight
    implicitWidth: paletteTree.implicitWidth

    enabled: paletteRootModel.paletteEnabled

    function applyCurrentPaletteElement() {
        paletteTree.applyCurrentElement();
    }

    color: ui.theme.backgroundPrimaryColor

    PaletteRootModel {
        id: paletteRootModel

        onPaletteSearchRequested: {
            palettesWidgetHeader.searchSelectAll()
        }
    }

    PalettesWidgetHeader {
        id: palettesWidgetHeader

        paletteWorkspace: palettesWidget.paletteWorkspace

        popupMaxHeight: palettesWidget.height * 0.8

        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 12
            right: parent.right
            rightMargin: 12
        }

        keynav.section: palettesWidget.keynavSection
        keynav.order: 2

        onAddCustomPaletteRequested: paletteTree.insertCustomPalette(0, paletteName);
    }

    StyledTextLabel {
        id: searchHint

        anchors.top: palettesWidgetHeader.bottom
        anchors.topMargin: 26
        anchors.horizontalCenter: parent.horizontalCenter

        text: qsTrc("palette", "Start typing to search all palettes")

        visible: palettesWidgetHeader.searchOpened && !Boolean(palettesWidgetHeader.searchText)
    }

    PaletteTree {
        id: paletteTree
        clip: true
        paletteWorkspace: palettesWidget.paletteWorkspace
        backgroundColor: palettesWidget.color

        keynav.section: palettesWidget.keynavSection
        keynav.order: 3
        keynav.enabled: paletteTree.visible

        filter: palettesWidgetHeader.searchText
        enableAnimations: !palettesWidgetHeader.searching
        searchOpened: palettesWidgetHeader.searchOpened

        anchors {
            top: palettesWidgetHeader.bottom
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
        visible: paletteRootModel.shadowOverlay
        anchors.fill: parent
        z: 1000

        color: ui.theme.strokeColor
        opacity: 0.5
    }
}
