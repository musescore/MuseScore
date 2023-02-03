/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.UiComponents 1.0
import Muse.Ui 1.0

ListItemBlank {
    id: root

    property var item: null
    property string keyRoleName: "key"
    property string valueRoleName: "value"
    property string valueTypeRole: "valueType"
    property string valueEnabledRoleName: "enabled"
    property string minValueRoleName: "min"
    property string maxValueRoleName: "max"
    property string iconRoleName: "icon"

    property bool readOnly: false

    property bool drawZebra: true
    property int keyColumnWidth: 0
    property bool isKeyEditable: false
    property bool startEditByDoubleClick: false

    property alias spacing: row.spacing
    property real sideMargin: 0
    property real valueItemWidth: 126

    signal keyEdited(string newKey)
    signal valueEdited(string newValue)
    signal escapeEdit()

    height: 34

    normalColor: drawZebra ? ( (index % 2 == 0) ? ui.theme.backgroundSecondaryColor : ui.theme.backgroundPrimaryColor ) : ui.theme.backgroundPrimaryColor

    navigation.accessible.name: root.item[keyRoleName] + ": " + (Boolean(valueLoader.item) ? valueLoader.item.accessibleName : "")

    navigation.onTriggered: {
        if (isKeyEditable) {
            keyLoader.item.startEdit(keyLoader.item.val)
        }
    }

    navigation.onActiveChanged: {
        if (isKeyEditable) {
            keyLoader.item.escaped()
        }
    }

    QtObject {
        id: privateProperties

        function componentByType(type) {
            switch (type) {
            case "Undefined": return textComp
            case "Bool": return boolComp
            case "Int": return intComp
            case "Double": return doubleComp
            case "String": return textComp
            case "Color": return colorComp
            }

            return textComp
        }

        function isNumberComponent() {
            return root.item[valueTypeRole] === "Int" || root.item[valueTypeRole] === "Double"
        }
    }

    onClicked: {
        forceActiveFocus()
    }

    RowLayout {
        id: row

        anchors.fill: parent

        RowLayout {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: root.keyColumnWidth != 0 ? false : true
            Layout.preferredWidth: root.keyColumnWidth != 0 ? root.keyColumnWidth : -1
            Layout.leftMargin: root.sideMargin

            spacing: icon.iconCode == IconCode.NONE ? 0 : 18

            StyledIconLabel {
                id: icon
                iconCode: Boolean(root.item[iconRoleName]) ? root.item[iconRoleName] : IconCode.NONE
            }

            Loader {
                id: keyLoader

                property var val: root.item[keyRoleName]

                Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                Layout.fillHeight: true
                Layout.fillWidth: true

                sourceComponent: root.isKeyEditable ? textComp : titleLabelComp

                onLoaded: {
                    keyLoader.item.val = keyLoader.val
                }

                onValChanged: {
                    if (keyLoader.item) {
                        keyLoader.item.val = keyLoader.val
                    }
                }

                Connections {
                    target: keyLoader.item
                    function onChanged(newVal) {
                        root.item[keyRoleName] = newVal
                        listItem.keyEdited(newVal)
                    }
                }
            }

            Component {
                id: titleLabelComp

                StyledTextLabel {
                    id: titleLabel

                    property string val

                    text: val

                    horizontalAlignment: Text.AlignLeft
                }
            }
        }

        Loader {
            id: valueLoader
            property var val: root.item[valueRoleName]
            property NavigationPanel navPanel: root.navigation.panel
            property int navRow: root.navigation.row
            property int navColumn: 1

            Layout.alignment: root.keyColumnWidth != 0 ? (Qt.AlignLeft | Qt.AlignVCenter) : (Qt.AlignRight | Qt.AlignVCenter)
            Layout.preferredWidth: root.keyColumnWidth != 0 ? -1 : root.valueItemWidth
            Layout.fillWidth: root.keyColumnWidth != 0 ? true : false
            Layout.rightMargin: root.sideMargin

            enabled: root.item[valueEnabledRoleName] !== undefined ? root.item[valueEnabledRoleName] : true

            sourceComponent: !root.readOnly ? privateProperties.componentByType(root.item[valueTypeRole]) : readOnlyComponent

            onLoaded: {
                valueLoader.item.val = valueLoader.val

                if (privateProperties.isNumberComponent() && !root.readOnly) {
                    if (Boolean(root.item[minValueRoleName])) {
                        valueLoader.item.minValue = root.item[minValueRoleName]
                    }

                    if (Boolean(root.item[maxValueRoleName])) {
                        valueLoader.item.maxValue = root.item[maxValueRoleName]
                    }
                }

                valueLoader.item.navPanel = valueLoader.navPanel
                valueLoader.item.navRow = valueLoader.navRow
                valueLoader.item.navColumn = valueLoader.navColumn
            }

            onValChanged: {
                if (valueLoader.item) {
                    valueLoader.item.val = valueLoader.val
                }
            }

            Connections {
                target: valueLoader.item
                function onChanged(newVal) {
                    root.item[valueRoleName] = newVal
                    listItem.valueEdited(newVal)
                }
            }
        }
    }

    SeparatorLine {
        anchors.bottom: row.bottom

        visible: !root.drawZebra
    }

    Component {
        id: textComp

        Loader {
            id: textLoader
            property var val
            signal changed(var newVal)
            signal startEdit(var val)
            signal escaped()

            property bool isKey: parent && parent === keyLoader

            property NavigationPanel navPanel: !isKey ? root.navigation.panel : null
            property int navRow: !isKey ? root.navigation.row : 0
            property int navColumn: !isKey ? 1 : 0

            sourceComponent: root.startEditByDoubleClick ? doubleClickTextComp : singleClickTextComp

            onLoaded: {
                textLoader.item.val = textLoader.val ?? ""

                textLoader.item.navPanel = textLoader.navPanel
                textLoader.item.navRow = textLoader.navRow
                textLoader.item.navColumn = textLoader.navColumn
            }

            onValChanged: {
                if (textLoader.item) {
                    textLoader.item.val = textLoader.val
                }
            }

            onStartEdit: function(val) {
                textLoader.item.startEdit(val)
            }

            onEscaped: {
                textLoader.item.escaped()
            }

            Connections {
                target: textLoader.item
                function onChanged(newVal) { textLoader.changed(newVal) }
            }
        }
    }

    Component {
        id: singleClickTextComp

        TextInputField {
            id: textControl

            property string val
            signal changed(string newVal)

            property string accessibleName: navigation.accessible.name
            property NavigationPanel navPanel: null
            property int navRow: 0
            property int navColumn: 0

            navigation.panel: navPanel
            navigation.row: navRow
            navigation.column: navColumn

            currentText: val

            onTextChanged: function(newTextValue) {
                textControl.changed(newTextValue)
            }
        }
    }

    Component {
        id: doubleClickTextComp

        Item {
            id: doubleClickItem

            height: root.height
            width: 10

            property string val
            property string accessibleName: navigation.accessible.name
            property NavigationPanel navPanel
            property int navRow
            property int navColumn

            signal changed(string newVal)
            signal startEdit(var val)
            signal escaped()

            onStartEdit: function(val) {
                valueEditLoader.edit(val)
            }

            onEscaped: {
                valueEditLoader.escaped()
            }

            NavigationFocusBorder {
                navigationCtrl: NavigationControl {
                    id: valueNavCtrl
                    enabled: doubleClickItem.enabled && doubleClickItem.visible
                    panel: doubleClickItem.navPanel
                    row: doubleClickItem.navRow
                    column: doubleClickItem.navColumn

                    onTriggered: {
                        valueEditLoader.edit(doubleClickItem.val)
                        root.clicked(mouseArea)
                    }

                    onActiveChanged: {
                        if (!active) {
                            valueEditLoader.escaped()
                        }
                    }
                }

                anchors.topMargin: 1
                anchors.bottomMargin: 1
            }

            StyledTextLabel {
                id: valueLabel

                anchors.fill: parent
                horizontalAlignment: Text.AlignLeft

                visible: !valueEditLoader.isEditState

                text: doubleClickItem.val
            }

            MouseArea {
                anchors.fill: valueLabel

                acceptedButtons: Qt.LeftButton
                hoverEnabled: true
                propagateComposedEvents: true

                onDoubleClicked: (mouse) => {
                    mouse.accepted = true
                    valueEditLoader.edit(doubleClickItem.val)
                }
            }

            Loader {
                id: valueEditLoader

                anchors.fill: valueLabel

                property bool isEditState: false
                sourceComponent: valueEditLoader.isEditState ? valueEditComp : null

                function edit(text) {
                    valueEditLoader.isEditState = true
                    valueEditLoader.item.currentText = text
                    valueEditLoader.item.newValue = text
                    valueEditLoader.item.visible = true
                    valueEditLoader.item.ensureActiveFocus()
                }

                function escaped() {
                    valueEditLoader.item.escaped()
                }
            }

            Component {
                id: valueEditComp

                TextInputField {
                    id: valueEdit

                    anchors.fill: parent

                    property string newValue: ""

                    background.color: "transparent"
                    background.border.width: 0
                    inputField.color: valueLabel.color
                    textSidePadding: 0
                    visible: false

                    onTextChanged: function (text) {
                        valueEdit.newValue = text
                    }

                    onAccepted: {
                        doubleClickItem.changed(valueEdit.newValue)
                        valueEditLoader.isEditState = false
                    }

                    onEscaped: {
                        valueEditLoader.isEditState = false
                    }

                    onFocusChanged: {
                        if (!valueEdit.focus) {
                            valueEdit.visible = false
                            valueEditLoader.isEditState = false
                            valueEdit.accepted()
                            doubleClickItem.changed(valueEdit.newValue)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: colorComp

        ColorPicker {
            id: colorControl

            property color val
            signal changed(color newVal)

            property string accessibleName: navigation.accessible.name

            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 1

            color: val
            allowAlpha: true

            onNewColorSelected: function(newColor) {
                colorControl.changed(newColor)
            }
        }
    }

    Component {
        id: intComp

        IncrementalPropertyControl {
            id: intControl

            property int val
            signal changed(int newVal)

            property string accessibleName: navigation.accessible.name

            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 1

            currentValue: val

            step: 1
            decimals: 0

            onValueEdited: function(newValue) {
                intControl.changed(newValue)
            }
        }
    }

    Component {
        id: doubleComp

        IncrementalPropertyControl {
            id: doubleControl

            property double val
            signal changed(double newVal)

            property string accessibleName: navigation.accessible.name

            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 1

            currentValue: val
            step: 1.0

            onValueEdited: function(newValue) {
                doubleControl.changed(newValue)
            }
        }
    }

    Component {
        id: boolComp

        CheckBox {
            id: boolControl

            property bool val
            signal changed(bool newVal)

            property string accessibleName: checked ? qsTrc("ui", "checked", "checkstate") : qsTrc("ui", "unchecked", "checkstate")

            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 1

            checked: val ? true : false
            onClicked: {
                boolControl.changed(!boolControl.checked)
            }
        }
    }

    Component {
        id: readOnlyComponent

        StyledTextLabel {
            property var val
            signal changed(var stub)

            property string accessibleName: text

            text: val
            horizontalAlignment: Text.AlignLeft
        }
    }
}
