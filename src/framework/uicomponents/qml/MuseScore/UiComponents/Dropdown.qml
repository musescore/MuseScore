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
    property string currentText: "--"
    property string currentValue: ""

    property string displayText : root.currentText

    property int popupWidth: root.width
    property int popupItemsCount: root.defaultPopupItemsCount()

    property alias dropIcon: dropIconItem
    property alias label: mainItem.label

    property alias navigation: mainItem.navigation

    signal activated(int index)

    height: 30
    width: 126

    //! NOTE We should not just bind to the current values, because when the component is created,
    //! the `onCurrentValueChanged` slot will be called, often in the handlers of which there are not yet initialized values
    Component.onCompleted: prv.updateCurrent()
    onCurrentIndexChanged: prv.updateCurrent()

    QtObject {
        id: prv

        function updateCurrent() {
            if (!(root.currentIndex >= 0 && root.currentIndex < root.count)) {
                root.currentText = "--"
                root.currentValue = ""
            }

            root.currentText = root.valueFromModel(root.currentIndex, root.textRole, "")
            root.currentValue = root.valueFromModel(root.currentIndex, root.valueRole, "")
        }
    }

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

    function defaultPopupItemsCount() {
        if (root.count > 20) {
            return 16
        }

        return 6
    }

    function positionViewAtFirstChar(text) {

        if (text === "") {
            return;
        }

        text = text.toLowerCase()
        var idx = -1
        for (var i = 0; i < root.count; ++i) {
            var itemText = accesser.itemAt(i).text
            if (itemText.toLowerCase().startsWith(text)) {
                idx = i;
                break;
            }
        }

        if (idx > -1) {
            view.positionViewAtIndex(idx, ListView.Center)
        }
    }

    function ensureActiveFocus() {
        if (mainItem.navigation) {
            mainItem.navigation.requestActive()
        }
    }

    DropdownItem {
        id: mainItem
        anchors.fill: parent
        text: root.displayText

        background.border.width: ui.theme.borderWidth
        background.border.color: ui.theme.strokeColor

        onClicked: {
            popup.navigationParentControl = root.navigation
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

    Popup {
        id: popup

        property NavigationControl navigationParentControl: null

        contentWidth: root.popupWidth
        contentHeight: root.height * Math.min(root.count, root.popupItemsCount)
        padding: ui.theme.borderWidth
        margins: 0

        x: 0
        y: 0

        onOpened: {
            popup.forceActiveFocus()
            contentItem.forceActiveFocus()
            view.positionViewAtIndex(root.currentIndex, ListView.Center)
            var item = view.itemAtIndex(root.currentIndex)
            item.navigation.requestActive()
        }

        function closeAndReturnFocus() {
            popup.close()
            popup.navigationParentControl.requestActive()
        }

        NavigationPanel {
            id: popupNavPanel
            name: root.navigation.name + "Popup"
            enabled: popup.opened
            direction: NavigationPanel.Vertical
            section: root.navigation.panel ? root.navigation.panel.section : null
            order: root.navigation.panel ? (root.navigation.panel.order + 1) : 0

            onActiveChanged: {
                if (popupNavPanel.active) {
                    popup.forceActiveFocus()
                    contentItem.forceActiveFocus()
                } else {
                    popup.closeAndReturnFocus()
                }
            }

            onNavigationEvent: {
                console.log("onNavigationEvent event: " + JSON.stringify(event))
                if (event.type === NavigationEvent.Escape) {
                    popup.closeAndReturnFocus()
                }
            }
        }

        background: Item {
            anchors.fill: parent

            Rectangle {
                id: bgItem
                anchors.fill: parent
                color: mainItem.background.color
                radius: 4
                border.width: ui.theme.borderWidth
                border.color: ui.theme.strokeColor
            }

            StyledDropShadow {
                anchors.fill: parent
                source: bgItem
            }
        }

        contentItem: FocusScope {
            id: contentItem
            focus: true

            Keys.onShortcutOverride: {
                // console.log("onShortcutOverride event: " + JSON.stringify(event))
                if (event.text !== "") {
                    event.accepted = true
                }

                if (event.key === Qt.Key_Escape) {
                    event.accepted = false
                }
            }

            Keys.onReleased: {
                // console.log("onReleased event: " + JSON.stringify(event))
                if (event.text === "") {
                    return
                }
                root.positionViewAtFirstChar(event.text)
            }

            Rectangle {
                anchors.fill: parent
                color: mainItem.background.color
                radius: 4

                ListView {
                    id: view

                    anchors.fill: parent
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    model: root.model

                    ScrollBar.vertical: StyledScrollBar {
                        anchors.right: parent.right
                        anchors.rightMargin: 4
                        width: 6
                    }

                    delegate: DropdownItem {

                        id: item

                        height: root.height
                        width: popup.contentWidth

                        insideDropdownList: true

                        navigation.name: item.text
                        navigation.panel: popupNavPanel
                        navigation.row: model.index
                        navigation.onActiveChanged: {
                            if (navigation.active) {
                                view.positionViewAtIndex(model.index, ListView.Contain)
                            }
                        }

                        background.radius: 0
                        background.opacity: 1.0
                        hoveredColor: ui.theme.accentColor

                        selected: model.index === root.currentIndex
                        text: root.valueFromModel(model.index, root.textRole, "")

                        onClicked: {
                            root.currentIndex = model.index
                            popup.closeAndReturnFocus()

                            root.activated(root.currentIndex)
                        }
                    }
                }
            }
        }
    }
}
