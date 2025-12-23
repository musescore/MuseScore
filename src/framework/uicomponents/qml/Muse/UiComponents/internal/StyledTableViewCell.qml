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

import Muse.UiComponents
import Muse.Ui 1.0

TableViewDelegate {
    id: root

    property var itemData: model.display

    property int cellType: TableViewCellType.Undefined
    property int cellEditMode: TableViewCellEditMode.StartInEdit
    property int preferredWidth: 0

    property var sourceComponentCallback

    property bool isSelected: false

    property alias navigation: listItem.navigation

    signal hoveredRequested(bool hovered)
    signal pressedRequested(bool pressed)
    signal selectedRequested(bool selected)

    signal editingStarted()
    signal editingFinished()

    implicitHeight: 44
    implicitWidth: Math.max(valueLoader.itemImplicitWidth, preferredWidth)

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    onPressedChanged: {
        prv.onCellPressedChanged(pressed)
    }

    onRowChanged: {
        if (Boolean(root.itemData)) {
            root.itemData.row = root.row
        }
    }

    onColumnChanged: {
        if (Boolean(root.itemData)) {
            root.itemData.column = root.column
        }
    }

    Component.onCompleted: {
        itemData.row = row
        itemData.column = column
    }

    QtObject {
        id: prv

        property bool isDoubleClickActivation: root.cellEditMode === TableViewCellEditMode.DoubleClick

        function onCellHoveredChanged(isHovered) {
            if (isHovered) {
                //! NOTE: Call with a delay so that
                // hovered = false of another cell is processed earlier
                Qt.callLater(doCellHoveredChanged, isHovered)
            } else {
                doCellHoveredChanged(isHovered)
            }
        }

        function doCellHoveredChanged(isHovered) {
            root.itemData.hovered = isHovered
        }

        function onCellPressedChanged(isPressed) {
            if (isPressed) {
                //! NOTE: Call with a delay so that
                // pressed = false of another cell is processed earlier
                Qt.callLater(doCellPressedChanged, isPressed)
            } else {
                doCellPressedChanged(isPressed)
            }
        }

        function doCellPressedChanged(isPressed) {
            root.itemData.pressed = isPressed
        }
    }

    contentItem: Loader {
        id: valueLoader

        property var itemData: root.itemData
        property var val: root.itemData.value
        property int row: root.row
        property int column: root.column

        property NavigationPanel navPanel: listItem.navigation.panel
        property int navRow: listItem.navigation.row
        property int navColumn: 1

        property int itemImplicitWidth: Boolean(item) ? anchors.leftMargin + item.implicitWidth + anchors.rightMargin : 0

        anchors.fill: parent
        anchors.leftMargin: column === 0 ? 16 : 8
        anchors.rightMargin: column === tableView.model.columnCount() - 1 ? 16 : 8
        anchors.topMargin: 8
        anchors.bottomMargin: 8

        sourceComponent: prv.isDoubleClickActivation ? doubleClickComponent : componentByType(root.cellType)

        function componentByType(type) {
            var comp = Boolean(root.sourceComponentCallback) ? root.sourceComponentCallback(type) : null
            if (Boolean(comp)) {
                return comp
            }

            switch (type) {
            case TableViewCellType.Undefined: return textComp
            case TableViewCellType.Bool: return boolComp
            case TableViewCellType.Int: return intComp
            case TableViewCellType.Double: return doubleComp
            case TableViewCellType.String: return textComp
            case TableViewCellType.List: return listComp
            case TableViewCellType.Color: return colorComp
            }

            return textComp
        }

        onLoaded: {
            if (prv.isDoubleClickActivation) {
                return
            }

            valueLoader.item.itemData = Qt.binding(function() { return root.itemData })
            valueLoader.item.val = Qt.binding(function() { return valueLoader.val })
            valueLoader.item.row = root.row
            valueLoader.item.column = root.column

            valueLoader.item.navigationPanel = listItem.navigation.panel
            valueLoader.item.navigationRow = listItem.navigation.row
            valueLoader.item.navigationColumnStart = listItem.navigation.column + 1
        }

        Connections {
            target: valueLoader.item
            function onChanged(newVal) {
                if (root.cellType === TableViewCellType.List) {
                    root.itemData.current = newVal
                } else {
                    root.itemData.value = newVal
                }
            }

            function onEditingFinished() {
                listItem.navigation.requestActive()

                root.editingFinished()
            }
        }

        Connections {
            target: root.itemData
            function onRequestEdit() {
                if (!prv.isDoubleClickActivation) {
                    return
                }

                valueLoader.item.edit()
            }
        }

        Component {
            id: textComp

            TextInputField {
                id: textControl

                property var itemData
                property string val
                property int row
                property int column

                property NavigationPanel navigationPanel
                property int navigationRow
                property int navigationColumnStart

                property string accessibleName: navigation.accessible.name

                signal editingFinished()
                signal changed(string newVal)

                navigation.panel: navigationPanel
                navigation.row: navigationRow
                navigation.column: navigationColumnStart

                currentText: valueLoader.val

                onTextEditingFinished: function(newTextValue){
                    textControl.changed(newTextValue)
                    textControl.editingFinished()
                }
            }
        }

        Component {
            id: colorComp

            ColorPicker {
                id: colorControl

                property var itemData
                property color val
                property int row
                property int column

                property NavigationPanel navigationPanel
                property int navigationRow
                property int navigationColumnStart

                property string accessibleName: navigation.accessible.name

                signal changed(color newVal)
                signal editingFinished()

                navigation.panel: navigationPanel
                navigation.row: navigationRow
                navigation.column: navigationColumnStart

                color: val
                allowAlpha: true

                onNewColorSelected: function(newColor) {
                    colorControl.changed(newColor)
                    colorControl.editingFinished()
                }
            }
        }

        Component {
            id: intComp

            IncrementalPropertyControl {
                id: intControl

                property var itemData
                property int val
                property int row
                property int column

                property NavigationPanel navigationPanel
                property int navigationRow
                property int navigationColumnStart

                property string accessibleName: navigation.accessible.name

                signal changed(int newVal)
                signal editingFinished()

                navigation.panel: navigationPanel
                navigation.row: navigationRow
                navigation.column: navigationColumnStart

                currentValue: val

                step: 1
                decimals: 0

                onValueEdited: function(newValue) {
                    intControl.changed(newValue)
                }

                onValueEditingFinished: {
                    intControl.editingFinished()
                }
            }
        }

        Component {
            id: doubleComp

            IncrementalPropertyControl {
                id: doubleControl

                property var itemData
                property double val
                property int row
                property int column

                property NavigationPanel navigationPanel
                property int navigationRow
                property int navigationColumnStart

                property string accessibleName: navigation.accessible.name

                signal changed(double newVal)
                signal editingFinished()

                navigation.panel: navigationPanel
                navigation.row: navigationRow
                navigation.column: navigationColumnStart

                currentValue: val
                step: 1.0

                onValueEdited: function(newValue) {
                    doubleControl.changed(newValue)
                }

                onValueEditingFinished: {
                    doubleControl.editingFinished()
                }
            }
        }

        Component {
            id: boolComp

            CheckBox {
                id: boolControl

                property var itemData
                property bool val
                property int row
                property int column

                property NavigationPanel navigationPanel
                property int navigationRow
                property int navigationColumnStart

                property string accessibleName: checked ? qsTrc("ui", "checked", "checkstate") : qsTrc("ui", "unchecked", "checkstate")

                signal changed(bool newVal)
                signal editingFinished()

                navigation.panel: navigationPanel
                navigation.row: navigationRow
                navigation.column: navigationColumnStart

                checked: val ? true : false
                onClicked: {
                    boolControl.changed(!boolControl.checked)
                    boolControl.editingFinished()
                }
            }
        }


        Component {
            id: listComp

            StyledDropdown {
                id: listControl

                property var itemData
                property var val
                property int row
                property int column

                property NavigationPanel navigationPanel
                property int navigationRow
                property int navigationColumnStart

                property string accessibleName: navigation.accessible.name

                signal changed(string newVal)
                signal editingFinished()

                model: val

                textRole: "value"
                currentIndex: root.itemData.current === "" ? 0 : indexOfValue(root.itemData.current)

                navigation.panel: navigationPanel
                navigation.row: navigationRow
                navigation.column: navigationColumnStart

                onActivated: function(index, value) {
                    listControl.changed(value)
                }

                onIsOpenedChanged: {
                    if (!isOpened) {
                        Qt.callLater(listControl.editingFinished)
                    }
                }
            }
        }

        Component {
            id: doubleClickComponent

            MouseArea {
                id: doubleClickControl

                property var itemData: valueLoader.itemData
                property var val: valueLoader.val
                property int row: valueLoader.row
                property int column: valueLoader.column

                property string accessibleName: doubleClickLoader.item ? doubleClickLoader.item.accessibleName : ""

                signal changed(var newVal)
                signal editingFinished()

                acceptedButtons: Qt.LeftButton
                propagateComposedEvents: true

                function edit() {
                    doubleClickLoader.edit()
                }

                onDoubleClicked: function(mouse) {
                    doubleClickLoader.edit(valueLoader.val)
                }

                onPressedChanged: function() {
                    prv.onCellPressedChanged(doubleClickControl.pressed)
                }

                Loader {
                    id: doubleClickLoader

                    anchors.fill: parent

                    property bool isEditState: false
                    sourceComponent: isEditState ? valueLoader.componentByType(root.cellType) : nonEditComp

                    function edit(text) {
                        doubleClickLoader.isEditState = true

                        Qt.callLater(doEdit)
                    }

                    function doEdit() {
                        doubleClickLoader.item.navigation.navigationEvent.connect(function(event) {
                            if (event.type === NavigationEvent.Escape) {
                                doubleClickLoader.isEditState = false
                                Qt.callLater(listItem.navigation.requestActive)
                            }
                        })

                        doubleClickLoader.item.navigation.requestActive()

                        root.editingStarted()
                    }

                    onLoaded: {
                        doubleClickLoader.item.itemData = Qt.binding(function() { return doubleClickControl.itemData })
                        doubleClickLoader.item.val = Qt.binding(function() { return doubleClickControl.val })
                        doubleClickLoader.item.row = doubleClickControl.row
                        doubleClickLoader.item.column = doubleClickControl.column

                        doubleClickLoader.item.navigationPanel = listItem.navigation.panel
                        doubleClickLoader.item.navigationRow = listItem.navigation.row
                        doubleClickLoader.item.navigationColumnStart = listItem.navigation.column + 1
                    }

                    Connections {
                        target: doubleClickLoader.item
                        function onChanged(newVal) {
                            doubleClickControl.changed(newVal)
                        }

                        function onEditingFinished() {
                            doubleClickLoader.isEditState = false
                            listItem.navigation.requestActive()

                            root.editingFinished()
                        }
                    }

                    Component {
                        id: nonEditComp

                        StyledTextLabel {
                            property var itemData
                            property string val
                            property int row
                            property int column

                            property NavigationPanel navigationPanel
                            property int navigationRow
                            property int navigationColumnStart

                            property string accessibleName: text

                            signal changed(string stub)
                            signal editingFinished()

                            text: root.cellType === TableViewCellType.List ? root.itemData.current : val
                            textFormat: Text.PlainText
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            }
        }
    }

    background: ListItemBlank {
        id: listItem

        anchors.fill: parent

        normalColor: ui.theme.backgroundPrimaryColor

        navigation.name: "TableViewCell"
        navigation.accessible.name: Boolean(valueLoader.item) ? valueLoader.item.accessibleName : ""
        navigation.accessible.role: MUAccessible.Cell

        navigation.onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.Trigger) {
                if (prv.isDoubleClickActivation) {
                    root.itemData.requestEdit()
                    event.accepted = true
                } else if (root.sourceComponentCallback) {
                    root.editingStarted()
                    Qt.callLater(valueLoader.item.navigation.requestActive)

                    event.accepted = true
                }
            }
        }

        onClicked: {
            forceActiveFocus()
            root.selectedRequested(!isSelected)
        }

        onHovered: function(isHovered, mouseX, mouseY) {
            prv.onCellHoveredChanged(isHovered)
        }

        onPressed: function(isPressed) {
            prv.onCellPressedChanged(isPressed)
        }

        SeparatorLine {
            anchors.bottom: parent.bottom
            orientation: Qt.Horizontal
        }
        SeparatorLine {
            anchors.right: parent.right
            orientation: Qt.Vertical
        }

        Component.onCompleted: {
            states = [ hoveredState, pressedState, selectedState, selectedHoveredState, selectedPressedState ]
        }

        states: []

        State {
            id: hoveredState

            name: "HOVERED"
            when: root.itemData.hovered && !root.itemData.pressed && !root.isSelected

            PropertyChanges {
                target: listItem.background
                opacity: ui.theme.buttonOpacityHover
                color: listItem.hoverHitColor
            }
        }

        State {
            id: pressedState

            name: "PRESSED"
            when: root.itemData.pressed && !listItem.isSelected

            PropertyChanges {
                target: listItem.background
                opacity: ui.theme.buttonOpacityHit
                color: listItem.hoverHitColor
            }
        }

        State {
            id: selectedState

            name: "SELECTED"
            when: !root.itemData.hovered && !root.itemData.pressed && root.isSelected

            PropertyChanges {
                target: listItem.background
                opacity: ui.theme.accentOpacityNormal
                color: ui.theme.accentColor
            }
        }

        State {
            id: selectedHoveredState

            name: "SELECTED_HOVERED"
            when: root.itemData.hovered && !root.itemData.pressed && root.isSelected

            PropertyChanges {
                target: listItem.background
                opacity: ui.theme.accentOpacityHover
                color: ui.theme.accentColor
            }
        }

        State {
            id: selectedPressedState

            name: "SELECTED_PRESSED"
            when: root.itemData.pressed && root.isSelected

            PropertyChanges {
                target: listItem.background
                opacity: ui.theme.accentOpacityHit
                color: ui.theme.accentColor
            }
        }
    }

    TableView.editDelegate: Item { }
}
