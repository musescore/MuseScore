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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RowLayout {
    id: root

    spacing: 12

    property bool separationGap: true

    default property alias customButtons : prv.customButtons

    property alias buttons: buttonBoxModel.buttons
    property int count: buttonBoxModel.rowCount()
    property var delegateProperties: null
    
    property NavigationPanel navigationPanel: null

    signal standardButtonClicked(int type)

    function accentButton() {
        var size = leftRepeater.count
        if (size === 0) {
            return null
        }

        for (var i = 0; i < size; i++) {
            var btn = leftRepeater.itemAt(i).visible ? leftRepeater.itemAt(i) : rightRepeater.itemAt(i)
            if (btn.accentButton) {
                return btn
            }
        }

        return leftRepeater.itemAt(0).visible ? leftRepeater.itemAt(0) : rightRepeater.itemAt(0)
    }

    function buttonClicked(type, customButtonIndex) {
        if (type === ButtonBoxModel.CustomButton) {
            prv.customButtons[customButtonIndex].clicked()
            return
        }
        standardButtonClicked(type)
    }

    function setDelegateButtonProperty(item, propertyName, propertyValue) {
        var subProperties = propertyName.split(".")

        if (subProperties.length === 1) {

            if (propertyName.length > 2 && propertyName.slice(0, 2) === "on") { //Changes "onClicked" to "clicked"
                var newPropertyName = propertyName.slice(2)
                newPropertyName = newPropertyName[0].toLowerCase() + newPropertyName.substring(1)
                item[newPropertyName].connect(function() { propertyValue(item) })

                return
            }

            item[propertyName] = propertyValue
        } else if (subProperties.length === 2) {

            if (subProperties[1].length > 2 && subProperties[1].slice(0, 2) === "on") { //Changes "onClicked" to "clicked"
                subProperties[1] = subProperties[1].slice(2)
                subProperties[1] = subProperties[1][0].toLowerCase() + subProperties[1].substring(1)
                item[subProperties[0]][subProperties[1]].connect(function() { propertyValue(item) })
                return
            }

            item[subProperties[0]][subProperties[1]] = propertyValue
        } else {
            console.log("Property: ", propertyName, " has more than two subproperties")
        }
    }

    function setDelegateButtonProperties(repeater) {
        if (!root.delegateProperties || root.delegateProperties.length === 0) {
            return
        }

        for (var i = 0; i < repeater.count; i++) {
            for (var propertyName in root.delegateProperties) {
                setDelegateButtonProperty(repeater.itemAt(i), propertyName, root.delegateProperties[propertyName])
            }
        }
    }

    QtObject {
        id: prv

        property list<ButtonBoxItem> customButtons
    }

    ButtonBoxModel {
        id: buttonBoxModel
    }

    Component.onCompleted: {
        var buttons = prv.customButtons
        for (var i = 0; i < buttons.length; i++) {
            var button = buttons[i]
            buttonBoxModel.addCustomButton(i, button.text, button.buttonRole, button.isAccent, button.isLeftSide, button.navigationName)
        }

        buttonBoxModel.load()
    }

    Repeater {
        id: leftRepeater

        model: buttonBoxModel
        delegate: FlatButton {
            text: model.text
            visible: model.isLeftSide && (model.type === ButtonBoxModel.CustomButton ?
                         prv.customButtons[model.customButtonIndex].visible : true)
            enabled: model.type === ButtonBoxModel.CustomButton ?
                         prv.customButtons[model.customButtonIndex].enabled : true

            accentButton: model.isAccent

            navigation.name: model.navigationName ? model.navigationName : model.text
            navigation.panel: root.navigationPanel
            navigation.column: model.navigationColumn

            onClicked: root.buttonClicked(model.type, model.customButtonIndex)
        }

        Component.onCompleted: root.setDelegateButtonProperties(leftRepeater)
    }

    Item {
        visible: root.separationGap
        Layout.fillWidth: true
    }

    Repeater {
        id: rightRepeater

        model: buttonBoxModel
        delegate: FlatButton {
            text: model.text
            visible: !model.isLeftSide && (model.type === ButtonBoxModel.CustomButton ?
                         prv.customButtons[model.customButtonIndex].visible : true)
            enabled: model.type === ButtonBoxModel.CustomButton ?
                         prv.customButtons[model.customButtonIndex].enabled : true

            accentButton: model.isAccent

            navigation.name: model.navigationName ? model.navigationName : model.text
            navigation.panel: root.navigationPanel
            navigation.column: model.navigationColumn

            onClicked: root.buttonClicked(model.type, model.customButtonIndex)
        }

        Component.onCompleted: root.setDelegateButtonProperties(rightRepeater)
    }
}
