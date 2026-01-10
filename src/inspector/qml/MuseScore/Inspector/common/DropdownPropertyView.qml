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
import QtQuick

import Muse.UiComponents
import Muse.Ui
import MuseScore.Inspector

InspectorPropertyView {
    id: root

    required property var model

    property Component dropdownComp: null
    property string dropdownTextRole: ""
    property string dropdownValueRole: ""

    navigationName: "DropdownPropertyView"
    navigationRowEnd: dropdownLoader.item.navigation.row

    function focusOnFirst() {
        dropdownLoader.item.navigation.requestActive()
    }

    Loader {
        id: dropdownLoader

        width: parent.width

        sourceComponent: root.dropdownComp ?? defaultDropdownComp

        onLoaded: {
            let dropdownComp = dropdownLoader.item
            if (!dropdownComp) {
                return
            }

            dropdownComp.width = Qt.binding(function() { return dropdownLoader.width })

            dropdownComp.navigation.name = Qt.binding(function() {
                return root.navigationName + " Dropdown"
            })

            dropdownComp.navigation.panel = Qt.binding(function() { return root.navigationPanel })

            dropdownComp.navigation.row = Qt.binding(function() { return root.navigationRowStart + 1 })

            dropdownComp.navigation.accessible.name = Qt.binding(function() {
                return root.accessibleName + " " + dropdownComp.currentText
            })

            dropdownComp.model = Qt.binding(function() { return root.model })

            dropdownComp.textRole = Qt.binding(function() {
                return root.dropdownTextRole ? root.dropdownTextRole : dropdownComp.textRole
            })
            dropdownComp.valueRole = Qt.binding(function() {
                return root.dropdownValueRole ? root.dropdownValueRole : dropdownComp.valueRole
            })

            dropdownComp.currentIndex = Qt.binding(function() {
                return root.propertyItem && !root.propertyItem.isUndefined
                    ? dropdownComp.indexOfValue(root.propertyItem.value)
                    : -1
            })
        }

        Connections {
            target: dropdownLoader.item

            function onActivated(index, value) {
                root.propertyItem.value = value
            }
        }
    }

    Component {
        id: defaultDropdownComp

        StyledDropdown { }
    }
}
