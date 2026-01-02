/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.UiComponents

BaseSection {
    id: root

    title: qsTrc("preferences", "Finale")

    property alias importPositionsTypes: importPositionsTypesBox.model
    property int importPositionsType: 0
    property alias convertTextSymbols: convertTextSymbolsBox.checked

    signal importPositionsTypeChangeRequested(int type)
    signal convertTextSymbolsChangeRequested(bool value)

    ComboBoxWithTitle {
        id: importPositionsTypesBox

        title: qsTrc("preferences", "When importing layout:")
        columnWidth: root.columnWidth

        currentIndex: control.indexOfValue(root.importPositionsType)

        control.textRole: "title"
        control.valueRole: "value"

        navigation.name: "importPositionsTypesBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onValueEdited: function(newIndex, newValue) {
            root.importPositionsTypeChangeRequested(newValue)
        }
    }

    CheckBox {
        id: convertTextSymbolsBox
        width: parent.width

        text: qsTrc("preferences", "Convert musical symbols to SMuFL symbols")

        navigation.name: "convertTextSymbolsBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.convertTextSymbolsChangeRequested(!checked)
        }
    }
}
