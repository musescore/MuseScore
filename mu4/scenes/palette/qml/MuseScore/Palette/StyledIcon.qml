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

import QtQuick 2.1
import QtGraphicalEffects 1.0

Item {
    property string source: ""
    property color color: ui.theme.fontPrimaryColor

    Image {
        id: img
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: parent.source
    }
    ColorOverlay {
        anchors.fill: img
        source: img
        color: parent.color
    }
}
