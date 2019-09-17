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
import MuseScore.Palette 3.3

Item {
    id: header

    property PaletteWorkspace paletteWorkspace: null
    property string cellFilter: searchTextInput.text
    readonly property bool searching: searchTextInput.activeFocus || searchTextClearButton.activeFocus

    signal addCustomPaletteRequested()

    implicitHeight: childrenRect.height

    StyledButton {
        id: morePalettesButton
        height: searchTextInput.height
        width: parent.width / 2 - 4
        text: qsTr("Add palettes")
        onClicked: palettePopup.visible = !palettePopup.visible
    }

    TextField {
        id: searchTextInput
        width: parent.width / 2 - 4
        anchors.right: parent.right

        placeholderText: qsTr("Search")
        //TODO: in the future we may wish these values to differ
        //Accessible.name: qsTr("Search")
        Accessible.name: placeholderText
        font: globalStyle.font

        color: globalStyle.text

        background: Rectangle {
            color: globalStyle.base
            border.color: "#aeaeae"
        }

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

            padding: 8

            text: qsTr("Clear search text")
            ToolTip.visible: hovered
            ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            ToolTip.text: text

            contentItem: StyledIcon {
                source: "icons/backspace.png"
            }
        }
    }

    PalettesListPopup {
        id: palettePopup
        paletteWorkspace: palettesWidget.paletteWorkspace

        visible: false

        y: morePalettesButton.y + morePalettesButton.height
        width: parent.width
        height: paletteTree.height * 0.8

        modal: true
        dim: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onAddCustomPaletteRequested: header.addCustomPaletteRequested()
    }

    Connections {
        target: palettesWidget
        onHasFocusChanged: {
            if (!palettesWidget.hasFocus && !palettePopup.inMenuAction)
                palettePopup.visible = false;
        }
    }
}
