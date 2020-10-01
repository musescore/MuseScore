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

CheckBox {
    id: checkbox
    font: globalStyle.font
    implicitHeight: 24

    property color textColor: globalStyle.buttonText
    property color visualFocusTextColor: globalStyle.brightText

    indicator.height: 24
    indicator.width: 24

    contentItem: Text {
        anchors.left: indicator.right
        anchors.leftMargin: 4
        text: checkbox.text
        font: checkbox.font
        opacity: checkbox.enabled ? 1 : 0.3
        color: checkbox.visualFocus ? checkbox.visualFocusTextColor : checkbox.textColor
        elide: Text.ElideRight
    }
}
