//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
import MuseScore.Palette 3.3
import MuseScore.Utils 3.3

StyledPopup {
    id: paletteOptionsPopup
    property PaletteWorkspace paletteWorkspace: null
    property var paletteModel: paletteTree.paletteModel
    property int numberOfExpandedPalettes: 0
    property int numberOfCollapsedPalettes: 0
    height: column.height + topPadding + bottomPadding

    property bool inMenuAction: false

    Column {
        id: column
        width: parent.width
        anchors.leftMargin: 0
        spacing: 4

        Text {
            id: header
            text: qsTr("Palette Options")
            font: globalStyle.font
            color: globalStyle.windowText
        }

        StyledCheckBox {
            id: singlePaletteCheckBox
            text: qsTr("Single Palette")
            anchors.left: parent.left
            width: parent.width
            checked: paletteWorkspace.singlePalette
            onCheckedChanged: paletteWorkspace.singlePalette = checked
            onClicked: paletteOptionsPopup.close()
            ToolTip.text: qsTr("Open only one palette at a time")
            Accessible.description: qsTr("Whether to open only one palette at a time")
        }

        ToolSeparator {
            orientation: Qt.Horizontal
            width: parent.width
        }

        StyledButton {
            id: collapseAllButton
            text: qsTr("Collapse All")
            Accessible.name: qsTr("Collapse all Palettes")
            width: parent.width

            enabled: numberOfExpandedPalettes > 0
            onClicked: {
                paletteTree.expandCollapseAll(false);
                paletteOptionsPopup.close();
            }
        }

        StyledButton {
            id: expandAllButton
            text: qsTr("Expand All")
            Accessible.name: qsTr("Expand all Palettes")
            width: parent.width

            enabled: numberOfCollapsedPalettes > 0 && !paletteWorkspace.singlePalette
            onClicked: {
                paletteTree.expandCollapseAll(true);
                paletteOptionsPopup.close();
            }
        }
    }

    onAboutToShow: {
        // TODO: change these properties automatically whenever a palette is expanded/collapsed
        numberOfExpandedPalettes = paletteTree.numberOfExpandedPalettes();
        numberOfCollapsedPalettes = paletteTree.numberOfCollapsedPalettes();
    }
}
