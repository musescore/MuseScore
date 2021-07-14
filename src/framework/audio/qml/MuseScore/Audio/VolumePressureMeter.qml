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
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Audio 1.0
import QtQuick.Controls 2.15

Canvas {
    id: root

    property real currentVolumePressure: -60.0
    property real minDisplayedVolumePressure: -60.0
    property real maxDisplayedVolumePressure: 0.0

    width: 6
    height: 140

    QtObject {
        id: internals

        property var gradient: null
        property int overloadHeight: 4
        property real pixelsPerUnit: height / minDisplayedVolumePressure
    }

    onPaint: {
        var ctx = root.context

        if (!ctx) {
            ctx = getContext("2d")
            ctx.translate(0, height)
            ctx.rotate(3 * (Math.PI/2))
        }

        ctx.clearRect(0, 0, height, width)

        ctx.fillStyle = "#4D4D4D"
        ctx.fillRect(0, 0, height - internals.overloadHeight, width)

        ctx.fillStyle = "#666666"
        ctx.fillRect(height - internals.overloadHeight, 0, height, width)

        if (!internals.gradient) {
            internals.gradient = ctx.createLinearGradient(0, 0, height - internals.overloadHeight, width)
            internals.gradient.addColorStop(0.0, "#26E386")
            internals.gradient.addColorStop(0.55, "#CBED41")
            internals.gradient.addColorStop(0.80, "#FC8226")
        }

        var value = height - (currentVolumePressure * internals.pixelsPerUnit)

        ctx.fillStyle = internals.gradient
        ctx.fillRect(0, 0, value, width)
    }

    onCurrentVolumePressureChanged: {
        requestPaint()
    }

    Component.onCompleted: {
        requestPaint()
    }
}
