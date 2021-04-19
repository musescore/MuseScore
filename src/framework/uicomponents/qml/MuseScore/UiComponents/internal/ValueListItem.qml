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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

ListItemBlank {
    id: root

    property var item: null
    property string keyRoleName: "key"
    property string valueRoleName: "value"
    property string valueTypeRole: "valueType"
    property string valueEnabledRoleName: "enabled"
    property string iconRoleName: "icon"

    property bool readOnly: false

    property alias spacing: row.spacing
    property real sideMargin: 0
    property real valueItemWidth: 126

    width: ListView.view.width
    height: 34

    normalStateColor: (index % 2 == 0) ? ui.theme.backgroundSecondaryColor
                                       : ui.theme.backgroundPrimaryColor

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

        Row {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.leftMargin: root.sideMargin

            spacing: 18

            StyledIconLabel {
                iconCode: Boolean(root.item[iconRoleName]) ? root.item[iconRoleName] : IconCode.NONE
            }

            StyledTextLabel {
                text: root.item[keyRoleName]
                horizontalAlignment: Text.AlignLeft
            }
        }

        Loader {
            id: loader
            property var val: root.item[valueRoleName]

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredWidth: root.valueItemWidth
            Layout.rightMargin: root.sideMargin

            enabled: root.item[valueEnabledRoleName] !== undefined ? root.item[valueEnabledRoleName] : true

            sourceComponent: !root.readOnly ? privateProperties.componentByType(root.item[valueTypeRole]) : readOnlyComponent

            onLoaded: {
                loader.item.val = loader.val

                if (privateProperties.isNumberComponent() && !root.readOnly) {
                    if (Boolean(root.item.min)) {
                        loader.item.minValue = root.item.min
                    }

                    if (Boolean(root.item.max)) {
                        loader.item.maxValue = root.item.max
                    }
                }
            }

            onValChanged: {
                if (loader.item) {
                    loader.item.val = loader.val
                }
            }

            Connections {
                target: loader.item
                function onChanged(newVal) {
                    root.item[valueRoleName] = newVal
                }
            }
        }
    }

    Component {
        id: textComp
        TextInputField {
            id: textControl

            property string val
            signal changed(string newVal)

            currentText: val

            onCurrentTextEdited: {
                textControl.changed(newTextValue)
            }
        }
    }

    Component {
        id: colorComp
        ColorPicker {
            id: colorControl

            property color val
            signal changed(color newVal)

            color: val

            onNewColorSelected: {
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

            iconMode: iconModeEnum.hidden

            currentValue: val

            step: 1
            validator: IntInputValidator {
                top: intControl.maxValue
                bottom: intControl.minValue
            }

            onValueEdited: {
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

            iconMode: iconModeEnum.hidden

            currentValue: val

            step: 1

            onValueEdited: {
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

            text: val
            horizontalAlignment: Text.AlignLeft
        }
    }
}
