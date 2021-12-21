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
.pragma library

function colorWithAlpha(color, alpha) {
    return Qt.rgba(color.r, color.g, color.b, alpha)
}

function colorToString(color) {
    var percentName = qsTrc("uicomponents", "percent")
    var colorValueToPercent = function(value) {
        return Math.floor(value * 100)
    }

    var colorValueTemplate = "%1 %2 %3 "
    var text = colorValueTemplate.arg(qsTrc("uicomponents", "Red")).arg(colorValueToPercent(color.r)).arg(percentName)
            + colorValueTemplate.arg(qsTrc("uicomponents", "Green")).arg(colorValueToPercent(color.g)).arg(percentName)
            + colorValueTemplate.arg(qsTrc("uicomponents", "Blue")).arg(colorValueToPercent(color.b)).arg(percentName)

    return text
}

function ensureContentVisible(flickable, contentRect, margins) {
    var flicableBottomY = flickable.contentY + flickable.height
    var contentBottomY = contentRect.y + contentRect.height

    var flicableTopY = flickable.contentY
    var contentTopY = contentRect.y

    if (flicableBottomY < contentBottomY) {
        flickable.contentY += contentBottomY - flicableBottomY + margins
    } else if (flicableTopY > contentTopY) {
        flickable.contentY -= flicableTopY - contentTopY + margins
    }
}

function getItem(model, row) {
    if (Boolean(model.get)) {
        return model.get(row)
    }

    return model[row]
}

function makeMnemonicText(text) {
    var index = text.indexOf("&")
    if (index !== -1) {
        if (index === text.lenght - 1) {
            text = removeAmpersands(text)
        } else {
            text = text.replace(text.substr(index, 2), ("<u>" + text.substr(index + 1, 1) + "</u>"))
        }
    }

    return text
}

function removeAmpersands(text) {
    return text.replace('&', '')
}
