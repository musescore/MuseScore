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
import QtQuick.Window 2.2
import MuseScore.Palette 3.3
import MuseScore.Utils 3.3

// TODO: make some properties 'property alias`?
// and `readonly property`?

Item {
    id: palettesWidget

    readonly property PaletteWorkspace paletteWorkspace: mscore.paletteWorkspace
    readonly property bool hasFocus: Window.activeFocusItem

    implicitHeight: 4 * palettesWidgetHeader.implicitHeight
    implicitWidth: paletteTree.implicitWidth

    enabled: mscore.palettesEnabled

    function applyCurrentPaletteElement() {
        paletteTree.applyCurrentElement();
    }

    function requestPaletteSearch () {
        palettesWidgetHeader.paletteSearchRequested()
    }

    FocusChainBreak { id: focusBreaker }

    PalettesWidgetHeader {
        id: palettesWidgetHeader

        paletteWorkspace: palettesWidget.paletteWorkspace

        anchors {
            top: parent.top
            topMargin: 6
            left: parent.left
            leftMargin: 12
            right: parent.right
            rightMargin: 12
        }

        onAddCustomPaletteRequested: paletteTree.insertCustomPalette(0);
    }

    ToolSeparator {
        id: separator
        orientation: Qt.Horizontal
        anchors.top: palettesWidgetHeader.bottom
        width: parent.width
    }

    PaletteTree {
        id: paletteTree
        clip: true
        paletteWorkspace: palettesWidget.paletteWorkspace

        filter: palettesWidgetHeader.cellFilter
        enableAnimations: !palettesWidgetHeader.searching

        anchors {
            top: separator.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    Rectangle {
        // Shadow overlay for Tours. The usual overlay doesn't cover palettes
        // as they reside in a window container above the main MuseScore window.
        visible: globalStyle.shadowOverlay
        anchors.fill: parent
        z: 1000

        color: globalStyle.shadow
        opacity: 0.5
    }
}
