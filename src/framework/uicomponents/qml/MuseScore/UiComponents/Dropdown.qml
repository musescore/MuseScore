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
import QtQml.Models 2.15

import MuseScore.Ui 1.0

import "internal"

Item {

    id: root

    property var model: null
    property alias count: view.count
    property string textRole: "text"
    property string valueRole: "value"

    property int currentIndex: 0

    readonly property string currentText: valueFromModel(root.currentIndex, root.textRole, root.indeterminateText)
    readonly property var currentValue: valueFromModel(root.currentIndex, root.valueRole, undefined)

    property string displayText : root.currentText
    property string indeterminateText: "--"

    property int popupWidth: root.width
    property int popupItemsCount: 18

    property alias dropIcon: dropIconItem
    property alias label: mainItem.label

    property alias navigation: mainItem.navigation

    signal activated(int index, var value)

    height: 30
    width: 126

    function valueFromModel(index, roleName, def) {
        if (!(index >= 0 && index < root.count)) {
            return def
        }

        // Simple models (like JS array) with single predefined role name - modelData
        if (root.model[index] !== undefined) {
            if (root.model[index][roleName] === undefined) {
                return root.model[index]
            }

            return root.model[index][roleName]
        }

        // Complex models (like QAbstractItemModel) with multiple role names
        if (!(index < delegateModel.count)) {
            return def
        }

        var item = delegateModel.items.get(index)
        return item.model[roleName]
    }

    function indexOfValue(value) {
        if (!root.model) {
            return -1
        }

        for (var i = 0; i < root.count; ++i) {
            if (root.valueFromModel(i, root.valueRole) === value) {
                return i
            }
        }

        return -1
    }

    function ensureActiveFocus() {
        if (mainItem.navigation) {
            mainItem.navigation.requestActive()
        }
    }

    QtObject {
        id: prv

        function itemIndexByFirstChar(text) {
            if (text === "") {
                return;
            }

            text = text.toLowerCase()
            for (var i = 0; i < root.count; ++i) {
                var itemText =  root.valueFromModel(i, root.textRole, "")
                if (itemText.toLowerCase().startsWith(text)) {
                    return i
                }
            }

            return -1
        }

        function positionViewAtFirstChar(text) {
            var index = itemIndexByFirstChar(text)

            if (index > -1) {
                positionViewAtIndex(index)
            }
        }

        function requestFocus() {
            positionViewAtIndex(root.currentIndex)
        }

        function positionViewAtIndex(itemIndex) {
            view.positionViewAtIndex(itemIndex, ListView.Center)
            Qt.callLater(navigateToItem, itemIndex)
        }

        function navigateToItem(itemIndex) {
            var item = view.itemAtIndex(itemIndex)
            item.navigation.requestActive()
        }
    }

    DropdownItem {
        id: mainItem
        anchors.fill: parent
        text: root.displayText

        navigation.accessible.role: MUAccessible.ComboBox

        background.border.width: ui.theme.borderWidth
        background.border.color: ui.theme.strokeColor

        onClicked: {
            popup.open()
        }
    }

    StyledIconLabel {
        id: dropIconItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 8

        iconCode: IconCode.SMALL_ARROW_DOWN
    }

    DelegateModel {
        id: delegateModel
        model: root.model
    }

    StyledPopupView {
        id: popup

        contentWidth: root.popupWidth
        contentHeight: root.height * Math.min(root.count, root.popupItemsCount)

        x: 0
        y: 0

        padding: 8
        margins: 0

        showArrow: false
        canOverrideParent: true

        onOpened: {
            prv.requestFocus()
        }

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "Dropdown"
            section: popup.navigationSection
            direction: NavigationPanel.Vertical
            order: 1
        }

        StyledListView {
            id: view

            anchors.fill: parent

            model: root.model

            scrollBarThickness: 6
            scrollBarPolicy: ScrollBar.AlwaysOn

            delegate: ListItemBlank {
                id: item

                height: root.height
                width: popup.contentWidth

                normalColor: ui.theme.buttonColor
                radius: 0

                isSelected: model.index === root.currentIndex

                navigation.name: label.text
                navigation.panel: popup.navigationPanel
                navigation.row: model.index
                navigation.onActiveChanged: {
                    if (navigation.highlight) {
                        view.positionViewAtIndex(model.index, ListView.Contain)
                    }
                }

                StyledTextLabel {
                    id: label
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    horizontalAlignment: Text.AlignLeft

                    text: root.valueFromModel(model.index, root.textRole, "")
                }

                onClicked: {
                    var newValue = root.valueFromModel(model.index, root.valueRole, undefined)
                    root.activated(model.index, newValue)

                    popup.close()
                }

                Keys.onShortcutOverride: function(event) {
                    if (event.text === "") {
                        event.accepted = false
                        return
                    }

                    if (prv.itemIndexByFirstChar(event.text) > -1) {
                        event.accepted = true
                    }
                }

                Keys.onPressed: function(event) {
                    if (event.text === "") {
                        return
                    }

                    if (prv.itemIndexByFirstChar(event.text) > -1) {
                        prv.positionViewAtFirstChar(event.text)
                    }
                }
            }
        }
    }
}
