/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.UiComponents 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "MusicXML")

    property alias importLayout: importLayoutBox.checked
    property alias importBreaks: importBreaksBox.checked
    property alias needUseDefaultFont: needUseDefaultFontBox.checked
    property alias inferTextType: inferTextTypeBox.checked

    signal importLayoutChangeRequested(bool importLayout)
    signal importBreaksChangeRequested(bool importBreaks)
    signal useDefaultFontChangeRequested(bool use)
    signal inferTextTypeChangeRequested(bool inferText)

    CheckBox {
        id: importLayoutBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Import layout")

        navigation.name: "ImportLayoutBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onClicked: {
            root.importLayoutChangeRequested(!checked)
        }
    }

    CheckBox {
        id: importBreaksBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Import system and page breaks")

        navigation.name: "ImportBreaksBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.importBreaksChangeRequested(!checked)
        }
    }

    CheckBox {
        id: needUseDefaultFontBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Apply default typeface (Edwin) to imported scores")

        navigation.name: "UseDefaultFontBox"
        navigation.panel: root.navigation
        navigation.row: 2

        onClicked: {
            root.useDefaultFontChangeRequested(!checked)
        }
    }

    CheckBox {
        id: inferTextTypeBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Infer text type based on content where possible")

        navigation.name: "InferTextTypeBox"
        navigation.panel: root.navigation
        navigation.row: 3

        onClicked: {
            root.inferTextTypeChangeRequested(!checked)
        }
    }
}
