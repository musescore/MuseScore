/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    //! NOTE: please, don't rename those properties because they are used in c++
    readonly property QtObject floatingWindowCpp: parent
    readonly property QtObject titleBarCpp: Boolean(floatingWindowCpp) ? floatingWindowCpp.titleBar : null
    readonly property QtObject dropAreaCpp: Boolean(floatingWindowCpp) ? floatingWindowCpp.dropArea : null
    readonly property int titleBarHeight: 0
    readonly property int margins: 8 // needed for the shadow

    anchors.fill: parent
    anchors.margins: margins

    onTitleBarHeightChanged: {
        if (Boolean(floatingWindowCpp)) {
            floatingWindowCpp.geometryUpdated()
        }
    }

    onDropAreaCppChanged: {
        if (Boolean(dropAreaCpp)) {
            dropAreaCpp.parent = dropArea
            dropAreaCpp.anchors.fill = dropArea
        }
    }

    Item {
        id: dropArea

        anchors.fill: parent
    }

    StyledDropShadow {
        anchors.fill: dropArea
        source: dropArea
        samples: 20
    }
}
