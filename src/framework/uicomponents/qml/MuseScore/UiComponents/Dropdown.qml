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

import MuseScore.Ui 1.0

import "internal"

DropdownItem {

    id: root

    property alias model: view.model
    property alias count: view.count

    property int currentIndex: 0
    property string currentText: root.valueFromModel(root.currentIndex, root.textRole)
    property string currentValue: root.valueFromModel(root.currentIndex, root.valueRole)

    property string displayText: root.currentText

    property int popupWidth: root.width
    property int popupItemsCount: 8

    property alias navigation: navCtrl

    property alias dropIcon: dropIconItem

    property string textRole: "text"
    property string valueRole: "value"

    signal activated(int index)

    text: root.displayText

    onClicked: {
        console.log("Dropdown.qml onClicked")
        popup.open()
    }

    function valueFromModel(index, roleName) {

        // Simple models (like JS array) with single predefined role name - modelData
        if (model[index] !== undefined) {
            if (model[index][roleName] === undefined) {
                return model[index]
            }

            return model[index][roleName]
        }

        // Complex models (like QAbstractItemModel) with multiple role names
        var item = delegateModel.items.get(index)

        return item.model[roleName]
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "Dropdown"
        enabled: root.enabled
        onActiveChanged: {
            if (!root.activeFocus) {
                root.forceActiveFocus()
            }
        }
        onTriggered: {
            if (popup.opened) {
                popup.close()
            } else {
                popup.open()
            }
        }
    }

    StyledIconLabel {
        id: dropIconItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 8

        iconCode: IconCode.SMALL_ARROW_DOWN
    }

    //StyledPopupView {
    Popup {
        id: popup

        contentWidth: root.popupWidth
        contentHeight: root.height * Math.min(root.count, root.popupItemsCount)
        padding: 0
        margins: 0
        //showArrow: false
        //background.color: "#CFD5DD"

        x: 0
        y: 0

        onOpened: {
            view.positionViewAtIndex(root.currentIndex, ListView.Center)
        }

        background: Rectangle {
            color: root.background.color
            radius: 4
        }

        contentItem: Rectangle {
            color: root.background.color
            radius: 4
            clip: true

            ListView {
                id: view

                anchors.fill: parent
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: StyledScrollBar {
                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    width: 4
                }

                delegate: DropdownItem {

                    height: root.height
                    width: popup.contentWidth

                    background.radius: 0
                    background.opacity: 1.0
                    hoveredColor: ui.theme.accentColor

                    selected: model.index === root.currentIndex
                    text: root.valueFromModel(model.index, root.textRole)

                    onClicked: {
                        root.currentIndex = model.index
                        root.activated(model.index)
                        popup.close()
                    }

                }
            }
        }
    }
}
