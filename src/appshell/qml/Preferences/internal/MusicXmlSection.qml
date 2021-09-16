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

BaseSection {
    id: root

    title: qsTrc("appshell", "MusicXML")

    property alias importLayout: importLayoutBox.checked
    property alias importBreaks: importBreaksBox.checked
    property alias needUseDefaultFont: needUseDefaultFontBox.checked

    signal importLayoutChangeRequested(bool importLayout)
    signal importBreaksChangeRequested(bool importBreaks)
    signal useDefaultFontChangeRequested(bool use)

    CheckBox {
        id: importLayoutBox

        width: 208
        text: qsTrc("appshell", "Import layout")

        onClicked: {
            root.importLayoutChangeRequested(!checked)
        }
    }

    CheckBox {
        id: importBreaksBox

        width: 208
        text: qsTrc("appshell", "Import system and page breaks")

        onClicked: {
            root.importBreaksChangeRequested(!checked)
        }
    }

    CheckBox {
        id: needUseDefaultFontBox

        width: 208
        text: qsTrc("appshell", "Apply default typeface (Edwin) to imported scores")

        onClicked: {
            root.useDefaultFontChangeRequested(!checked)
        }
    }
}
