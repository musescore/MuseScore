/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.7

Rectangle {

    id: root

    color: "#0F9D58"

    Loader {
        id: windowLoader
        anchors.fill: parent
        onStatusChanged: {
            if (item && item.anchors) item.anchors.fill = item ? item.parent : null
            item.visible = true
        }
    }

    Component.onCompleted: {
        var comp = Qt.createComponent("Window.qml");
        if (comp.status !== Component.Ready) {
            console.debug("qml: show window error: " + comp.errorString())
        }
        windowLoader.sourceComponent = comp
    }

}
