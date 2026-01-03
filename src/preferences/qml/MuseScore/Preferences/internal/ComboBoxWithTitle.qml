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

import Muse.UiComponents

Row {
    id: root

    readonly property bool isOpened: comboBoxLoader.item.isOpened

    property alias title: titleLabel.text

    property real columnWidth: 208

    property int currentIndex: -1
    property var currentValue: null
    property var model: null

    property string navigationName: ""
    property NavigationPanel navigationPanel: null
    property int navigationRow: -1
    property int navigationColumn: -1

    property Component dropdownComp: null
    property string textRole: "text"
    property string valueRole: "value"
    property int controlWidth: -1

    signal valueEdited(int newIndex, var newValue)

    spacing: 12

    function indexOfValue(value) {
        return comboBoxLoader.item.indexOfValue(value)
    }

    StyledTextLabel {
        id: titleLabel

        width: root.columnWidth
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Qt.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }

    Loader {
        id: comboBoxLoader

        sourceComponent: root.dropdownComp ?? defaultDropdownComp

        onLoaded: {
            let dropdown = comboBoxLoader.item
            if (!dropdown) {
                return
            }

            dropdown.width = Qt.binding(function() {
                return root.controlWidth > 0 ? root.controlWidth : root.columnWidth
            })

            dropdown.navigation.name = Qt.binding(function () { return root.navigationName })
            dropdown.navigation.panel = Qt.binding(function () { return root.navigationPanel })
            dropdown.navigation.row = Qt.binding(function () { return root.navigationRow })
            dropdown.navigation.column = Qt.binding(function () { return root.navigationColumn })
            dropdown.navigation.accessible.name = Qt.binding(function() {
                return root.title + " " + dropdown.currentText
            })

            dropdown.indeterminateText = ""

            dropdown.textRole = Qt.binding(function() { return root.textRole })
            dropdown.valueRole = Qt.binding(function() { return root.valueRole })
            dropdown.model = Qt.binding(function() { return root.model })
            dropdown.currentIndex = Qt.binding(function () { return root.currentIndex })
            dropdown.currentValue = Qt.binding(function () { return root.currentValue })
        }

        Connections {
            target: comboBoxLoader.item

            function onActivated(index, value) {
                root.valueEdited(index, value)
            }
        }
    }

    Component {
        id: defaultDropdownComp

        StyledDropdown { }
    }
}
