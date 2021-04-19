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
import QtQuick 2.12
import MuseScore.Ui 1.0

StyledTextLabel {
    id: root

    property int iconCode: IconCode.NONE
    readonly property bool isEmpty: iconCode === IconCode.NONE

    height: isEmpty ? 0 : implicitHeight
    width: isEmpty ? 0 : implicitWidth

    font {
        family: ui.theme.iconsFont.family
        pixelSize: ui.theme.iconsFont.pixelSize
    }

    text: iconCharCode(iconCode)

    function iconCharCode(code) {
        var result = 0

        switch (code) {
        case IconCode.AUTO: result = "AUTO"; break
        case IconCode.NONE: result = ""; break
        case IconCode.CUSTOM: result = "Custom"; break
        default: result = String.fromCharCode(code); break
        }

        return result
    }
}
