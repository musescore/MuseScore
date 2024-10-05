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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "internal"

Item {
    id: root

    property var model: null
    property int count: Boolean(model) ? model.length : 0
    property string textRole: "text"
    property string valueRole: "value"

    property int currentIndex: -1

    property string currentText: Utils.getItemValue(model, currentIndex, textRole, indeterminateText)
    property var currentValue: Utils.getItemValue(model, currentIndex, valueRole, undefined)

    property string displayText : root.currentText
    property string indeterminateText: "--"

    property int popupItemsCount: 18

    property alias dropIcon: mainItem.dropIcon
    property alias label: mainItem.label

    property alias navigation: mainItem.navigation

    signal activated(int index, var value)

    height: 30
    width: 126

    function indexOfValue(value) {
        if (!root.model) {
            return -1
        }

        for (var i = 0; i < root.count; ++i) {
            if (Utils.getItemValue(root.model, i, root.valueRole) === value) {
                return i
            }
        }

        return -1
    }

    function indexOfText(text) {
        if (!root.model) {
            return -1
        }

        for (var i = 0; i < root.count; ++i) {
            if (Utils.getItemValue(root.model, i, root.textRole) === text) {
                return i
            }
        }

        return -1
    }

    function textOfValue(value) {
        if (!root.model) {
            return ""
        }

        for (var i = 0; i < root.count; ++i) {
            if (Utils.getItemValue(root.model, i, root.valueRole) === value) {
                return Utils.getItemValue(model, i, textRole, indeterminateText)
            }
        }

        return ""
    }

    function ensureActiveFocus() {
        if (mainItem.navigation) {
            mainItem.navigation.requestActive()
        }
    }

    Item {
        id: mainItem

        anchors.fill: parent

        property bool selected: false
        property bool insideDropdownList: false

        property alias label: labelItem
        property alias dropIcon: dropIconItem

        property color hoveredColor: backgroundItem.color

        property alias navigation: navCtrl

        NavigationControl {
            id: navCtrl

            name: mainItem.objectName != "" ? mainItem.objectName : "Dropdown"
            enabled: mainItem.enabled && mainItem.visible
            accessible.role: MUAccessible.ComboBox
            accessible.name: labelItem.text

            onActiveChanged: {
                if (!mainItem.activeFocus) {
                    mainItem.forceActiveFocus()
                }
            }

            onTriggered: mainItem.clicked()
        }

        Rectangle {
            id: backgroundItem
            anchors.fill: parent
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
            color: ui.theme.buttonColor
            radius: 3
            opacity: 0.7

            NavigationFocusBorder { navigationCtrl: navCtrl }
        }

        StyledTextLabel {
            id: labelItem
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: dropIconItem.left
            anchors.leftMargin: 12
            anchors.rightMargin: 6
            horizontalAlignment: Text.AlignLeft
            text: root.displayText
        }

        StyledIconLabel {
            id: dropIconItem
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 8

            iconCode: IconCode.SMALL_ARROW_DOWN
        }

        MouseArea {
            id: mouseAreaItem
            anchors.fill: parent

            enabled: mainItem.enabled
            hoverEnabled: true

            onClicked: mainItem.clicked()

            onContainsMouseChanged: {
                if (!labelItem.truncated) {
                    return
                }

                if (mouseAreaItem.containsMouse) {
                    ui.tooltip.show(mainItem, labelItem.text)
                } else {
                    ui.tooltip.hide(mainItem)
                }
            }
        }

        function clicked() {
            dropdownLoader.toggleOpened(root.model)
        }

        StyledDropdownLoader {
            id: dropdownLoader

            textRole: root.textRole
            valueRole: root.valueRole

            currentIndex: root.currentIndex

            itemWidth: mainItem.width
            itemHeight: mainItem.height

            visibleItemsCount: root.popupItemsCount

            onHandleItem: function(index, value) {
                root.activated(index, value)
            }
        }

        states: [
            State {
                name: "FOCUSED_INSIDE_DROPDOWN"
                when: mainItem.insideDropdownList && navCtrl.active

                PropertyChanges {
                    target: backgroundItem
                    anchors.margins: ui.theme.navCtrlBorderWidth //this effectively cancels its child's margins and draws everything inside
                }
            },

            State {
                name: "HOVERED"
                when: mouseAreaItem.containsMouse && !mouseAreaItem.pressed

                PropertyChanges {
                    target: backgroundItem
                    opacity: ui.theme.buttonOpacityHover
                    color: mainItem.hoveredColor
                }
            },

            State {
                name: "PRESSED"
                when: mouseAreaItem.pressed

                PropertyChanges {
                    target: backgroundItem
                    opacity: ui.theme.buttonOpacityHit
                }
            },

            State {
                name: "SELECTED"
                when: mainItem.selected

                PropertyChanges {
                    target: backgroundItem
                    opacity: ui.theme.accentOpacityHit
                    color: ui.theme.accentColor
                }
            }
        ]
    }
}
