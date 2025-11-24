/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Engraving

ColumnLayout {
    id: root
    objectName: "DiagnosticEngravingStylePanel"

    EngravingStyleModel {
        id: styleModel
    }

    Component.onCompleted: {
        styleModel.init()
    }

    SearchField {
        id: searchField
    }

    StyledListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 2

        model: SortFilterProxyModel {
            id: sortFilterModel
            sourceModel: styleModel

            filters: [
                FilterValue {
                    roleName: "titleRole"
                    roleValue: searchField.searchText
                    compareType: CompareType.Contains
                }
            ]

            sorters: [
                SorterValue {
                    roleName: "keyRole"
                    sortOrder: Qt.AscendingOrder
                    enabled: true
                }
            ]
        }

        delegate: Item {
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            height: ui.theme.defaultButtonSize

            StyledTextLabel {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: control.left
                horizontalAlignment: Qt.AlignLeft

                text: titleRole
            }

            Item {
                id: control
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: resetButton.left
                anchors.rightMargin: 8
                width: 150

                Loader {
                    id: loader
                    property var val: valueRole
                    anchors.fill: parent
                    sourceComponent: root.componentByType(typeRole)
                    onLoaded: {
                        item.val = val
                    }

                    onValChanged: {
                        if (item) {
                            item.val = val
                        }
                    }
                }

                Connections {
                    target: loader.item
                    function onChanged(newVal) {
                        styleModel.changeVal(keyRole, newVal)
                    }
                }
            }

            FlatButton {
                id: resetButton
                icon: IconCode.UNDO
                toolTipTitle: "Reset " + titleRole
                enabled: isDefaultRole === false
                width: height
                anchors.right: parent.right

                onClicked: {
                    styleModel.changeVal(keyRole, undefined)
                }
            }
        }
    }

    function componentByType(type) {
        switch (type) {
        case "Undefined": return textComp;
        case "Bool":      return boolComp;
        case "Int":       return intComp;
        case "Double":    return doubleComp;
        case "String":    return textComp;
        case "Color":     return colorComp;
        case "Point":     return pointComp;
        case "Size":      return sizeComp;
        }

        return textComp;
    }

    Component {
        id: textComp

        TextInputArea {
            property var val
            signal changed(var newVal)
            anchors.fill: parent
            textSidePadding: 0
            currentText: String(val)
            onTextEditingFinished: newText => changed(newText)
        }
    }

    Component {
        id: colorComp

        ColorPicker {
            property var val
            signal changed(var newVal)

            anchors.fill: parent
            color: val

            onNewColorSelected: newColor => changed(newColor)
        }
    }

    Component {
        id: intComp

        IncrementalPropertyControl {
            property int val
            signal changed(var newVal)
            anchors.centerIn: parent

            currentValue: val
            minValue: -1000
            maxValue: 1000

            step: 1
            decimals: 0

            onValueEdited: newValue => changed(newValue)
        }
    }

    Component {
        id: doubleComp

        IncrementalPropertyControl {
            property real val
            signal changed(var newVal)
            anchors.centerIn: parent

            currentValue: val
            minValue: -1000
            maxValue: 1000

            step: 0.1
            decimals: 5

            onValueEdited: newValue => changed(newValue)
        }
    }

    Component {
        id: boolComp

        CheckBox {
            property var val
            signal changed(var newVal)
            anchors.centerIn: parent
            checked: Boolean(val)
            onClicked: changed(!checked)
        }
    }

    Component {
        id: pointComp

        Item {
            id: pointItem
            anchors.fill: parent
            property var val
            signal changed(var newVal)

            IncrementalPropertyControl {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4
                prefixIcon: IconCode.HORIZONTAL
                minValue: -1000
                maxValue: 1000
                step: 0.1
                decimals: 5

                currentValue: pointItem.val.x
                onValueEdited: function(newValue) {
                    let p = pointItem.val
                    p.x = newValue
                    pointItem.changed(p)
                }
            }

            IncrementalPropertyControl {
                anchors.left: parent.horizontalCenter
                anchors.right: parent.right

                prefixIcon: IconCode.VERTICAL
                minValue: -1000
                maxValue: 1000
                step: 0.1
                decimals: 5

                currentValue: pointItem.val.y
                onValueEdited: function(newValue) {
                    let p = pointItem.val
                    p.y = newValue
                    pointItem.changed(p)
                }
            }
        }
    }

    Component {
        id: sizeComp

        Item {
            id: sizeItem
            anchors.fill: parent
            property var val
            signal changed(var newVal)

            IncrementalPropertyControl {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4
                prefixIcon: IconCode.HORIZONTAL
                minValue: -1000
                maxValue: 1000
                step: 0.1
                decimals: 5

                currentValue: sizeItem.val.width
                onValueEdited: function(newValue) {
                    let p = sizeItem.val
                    p.width = newValue
                    sizeItem.changed(p)
                }
            }

            IncrementalPropertyControl {
                anchors.left: parent.horizontalCenter
                anchors.right: parent.right

                prefixIcon: IconCode.VERTICAL
                minValue: -1000
                maxValue: 1000
                step: 0.1
                decimals: 5

                currentValue: sizeItem.val.height
                onValueEdited: function(newValue) {
                    let p = sizeItem.val
                    p.height = newValue
                    sizeItem.changed(p)
                }
            }
        }
    }
}
