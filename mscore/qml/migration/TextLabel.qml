//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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

import QtQuick 2.5

FocusableItem {
    id: root

    property alias color: textLabel.color
    property alias lineHeight: textLabel.lineHeight
    property alias wrapMode: textLabel.wrapMode
    property alias text: textLabel.text
    property alias horizontalAlignment: textLabel.horizontalAlignment
    property alias verticalAlignment: textLabel.verticalAlignment
    property alias font: textLabel.font

    signal linkActivated()

    height: textLabel.height
    width: parent.width

    Accessible.role: Accessible.StaticText
    Accessible.name: textLabel.text

    Text {
        id: textLabel

        height: implicitHeight
        width: parent.width

        font.pixelSize: 12
        horizontalAlignment: Text.AlignHCenter

        wrapMode: Text.WordWrap

        color: globalStyle.buttonText

        onLinkActivated: {
            root.linkActivated()
        }
    }
}
