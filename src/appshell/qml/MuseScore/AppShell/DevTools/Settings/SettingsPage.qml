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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import Muse.Dock
import MuseScore.AppShell

ColumnLayout {
    id: root

    SettingListModel {
        id: settingsModel
    }

    Component.onCompleted: {
        settingsModel.load()
    }

    SearchField {
        id: searchField
    }

    StyledListView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        model: SortFilterProxyModel {
            id: sortFilterModel
            sourceModel: settingsModel

            filters: [
                FilterValue {
                    roleName: "keyRole"
                    roleValue: searchField.searchText
                    compareType: CompareType.Contains
                }
            ]

            sorters: [
                SorterValue {
                    roleName: "sectionRole"
                    sortOrder: Qt.AscendingOrder
                    enabled: true
                }
            ]
        }

        section.property: "sectionRole"
        section.delegate: Rectangle {
            id: sectionItem

            required property string section

            width: parent.width
            height: 24
            color: ui.theme.backgroundSecondaryColor

            StyledTextLabel {
                anchors.fill: parent
                anchors.margins: 2
                horizontalAlignment: Qt.AlignLeft
                text: sectionItem.section
            }
        }

        delegate: Item {
            id: delegateItem

            required property string keyRole
            required property string typeRole
            required property var valueRole
            required property real minValueRole
            required property real maxValueRole
            required property int index

            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            height: 32

            StyledTextLabel {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: control.left
                horizontalAlignment: Qt.AlignLeft

                text: delegateItem.keyRole
            }

            Item {
                id: control
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 2
                width: 150

                Loader {
                    id: loader
                    property var val: delegateItem.valueRole
                    property var minValue: delegateItem.minValueRole
                    property var maxValue: delegateItem.maxValueRole
                    anchors.fill: parent
                    sourceComponent: root.componentByType(delegateItem.typeRole)
                    onLoaded: {
                        loader.item.val = loader.val

                        if (loader.item.minValue !== undefined) {
                            loader.item.minValue = loader.minValue
                        }

                        if (loader.item.maxValue !== undefined) {
                            loader.item.maxValue = loader.maxValue
                        }
                    }

                    onValChanged: {
                        if (loader.item) {
                            loader.item.val = loader.val
                        }
                    }
                }

                Connections {
                    target: loader.item
                    function onChanged(newVal) {
                        let sortFilterModelIndex = sortFilterModel.index(delegateItem.index, 0);
                        let sourceModelIndex = sortFilterModel.mapToSource(sortFilterModelIndex)
                        settingsModel.changeVal(sourceModelIndex.row, newVal)
                    }
                }
            }
        }
    }

    function componentByType(type) {
        switch (type) {
        case "Undefined": return textComp;
        case "Bool": return boolComp;
        case "Int": return intComp;
        case "Double": return doubleComp;
        case "String": return textComp;
        case "Color": return colorComp;
        }

        return textComp;
    }

    Component {
        id: textComp

        Rectangle {
            id: textControl
            property var val
            signal changed(var newVal)
            anchors.fill: parent
            border.width: 1
            border.color: ui.theme.strokeColor

            TextEdit {
                anchors.fill: parent
                anchors.margins: 2
                verticalAlignment: Text.AlignVCenter
                text: String(textControl.val)
                onEditingFinished: textControl.changed(text)
            }
        }
    }

    Component {
        id: colorComp
        ColorPicker {
            property var val
            signal changed(var newVal)

            anchors.fill: parent
            color: val
            allowAlpha: true

            onNewColorSelected: function(newColor) {
                changed(newColor)
            }
        }
    }

    Component {
        id: intComp

        IncrementalPropertyControl {
            property int val

            signal changed(var newVal)
            anchors.centerIn: parent

            currentValue: val

            step: 1
            decimals: 0

            onValueEdited: function(newValue) {
                changed(newValue)
            }
        }
    }

    Component {
        id: doubleComp

        IncrementalPropertyControl {
            property real val

            signal changed(var newVal)
            anchors.centerIn: parent

            currentValue: val

            step: 1
            decimals: 2

            onValueEdited: function(newValue) {
                changed(newValue)
            }
        }
    }

    Component {
        id: boolComp
        CheckBox {
            id: checkbox
            property var val
            signal changed(var newVal)
            anchors.fill: parent
            checked: val ? true : false
            onClicked: checkbox.changed(!checkbox.checked)
        }
    }
}
