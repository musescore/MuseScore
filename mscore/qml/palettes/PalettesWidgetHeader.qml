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

    implicitHeight: childrenRect.height

    states: [
        State {
            name: "default"
            PropertyChanges { target: searchTextInput; text: ""; restoreEntryValues: false }
        },

        State {
            name: "searchExpanded"
//             PropertyChanges { target: morePalettesButton; visible: false }
            PropertyChanges { target: searchButton; visible: false }
            PropertyChanges { target: searchTextInput; /*visible: true; */focus: true }
        }
    ]

    state: "default"
    property int animationDuration: 150

    transitions: [
        Transition {
            from: "default"
            to: "searchExpanded"
            SequentialAnimation {
                PropertyAction { target: searchTextInput; property: "visible"; value: true }
                NumberAnimation { target: searchTextInput; property: "width"; from: 0; to: header.width; duration: animationDuration }
            }
            SequentialAnimation {
                NumberAnimation { target: morePalettesButton; property: "opacity"; from: 1.0; to: 0.0; easing.type: Easing.OutCubic; duration: animationDuration }
                PropertyAction { target: morePalettesButton; property: "visible"; value: false }
            }
        },
        Transition {
            from: "searchExpanded"
            to: "default"
            SequentialAnimation {
                NumberAnimation { target: searchTextInput; property: "width"; to: 0; duration: animationDuration }
                PropertyAction { target: searchTextInput; property: "visible"; value: false }
            }
            SequentialAnimation {
                PropertyAction { target: morePalettesButton; property: "visible"; value: true }
                NumberAnimation { target: morePalettesButton; property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.OutCubic; duration: animationDuration }
            }
        }
    ]

    Button {
        id: morePalettesButton
        text: qsTr("Add more palettes")
        onClicked: palettePopup.visible = !palettePopup.visible
    }

    TextField {
        id: searchTextInput
        visible: false
        anchors.right: parent.right

        placeholderText: qsTr("Search palettes")

        Keys.onEscapePressed: header.state = "default"
    }

    Button {
        id: searchButton
        flat: true
        anchors.right: parent.right

        height: searchTextInput.height
        width: height

        // Button.icon property is unavailable until Qt 5.10
        contentItem: Image {
            fillMode: Image.PreserveAspectFit
            source: "icons/search.png"
            // TODO: add ColorOverlay to get a correct appearance in a dark theme, see https://forum.qt.io/topic/75792/qtquick-controls-2-button-icon/5
        }

        onClicked: header.state = "searchExpanded"
    }

    PalettesListPopup {
        id: palettePopup
        paletteWorkspace: palettesWidget.paletteWorkspace

        visible: false

        y: morePalettesButton.y + morePalettesButton.height
        width: parent.width
        height: paletteTree.height * 0.8

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    }

    Connections {
        target: palettesWidget
        onHasFocusChanged: {
            if (!palettesWidget.hasFocus)
                palettePopup.visible = false;
        }
    }
}
