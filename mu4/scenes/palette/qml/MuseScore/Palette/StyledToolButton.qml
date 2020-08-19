//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

ToolButton {
    id: button
//     implicitHeight: 24
    flat: true

    property color textColor: ui.theme.fontPrimaryColor
    property color visualFocusTextColor: ui.theme.accentColor // TODO globalStyle.brightText

//     contentItem: Text {
//         text: button.text
//         font: button.font
//         opacity: button.enabled ? 1 : 0.3
//         color: button.visualFocus ? button.visualFocusTextColor : button.textColor
//         verticalAlignment: Text.AlignVCenter
//         horizontalAlignment: Text.AlignHCenter
//         elide: Text.ElideRight
//     }

    background: Rectangle {
        visible: !button.flat || button.hovered || button.down || button.visualFocus
        implicitWidth: 24
        implicitHeight: 24
        color: button.highlighted || button.visualFocus
            ? (button.down || button.hovered ? ui.theme.button : Qt.lighter(ui.theme.buttonColor, 1.2))
            : (button.down || button.hovered ? Qt.darker(ui.theme.buttonColor, 1.2) : ui.theme.buttonColor)
        opacity: button.hovered && !button.down ? 0.5 : 1.0
//         border {
//             color: ui.theme.buttonText
//             width: 1
//         }
    }
}
