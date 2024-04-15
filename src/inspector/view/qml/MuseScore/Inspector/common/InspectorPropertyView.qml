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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

Column {
    id: root

    property PropertyItem propertyItem: null

    property string navigationName: "InspectorPropertyView"
    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    property int buttonNavigationRow: navigationRowStart

    // Normally, this is just the reset or menu button (navigationRowStart + 0),
    // and one control (navigationRowStart + 1)
    property int navigationRowEnd: navigationRowStart + 1

    property string titleText: ""
    property Component titleLabelComponent: null
    property alias showTitle: titleLabelLoader.visible
    property alias showButton: buttonLoader.visible

    readonly property bool isStyled: propertyItem ? propertyItem.isStyled : false
    property bool isModified: propertyItem ? propertyItem.isModified : false

    signal requestResetToDefault()
    signal requestApplyToStyle()

    function requestActiveFocus() {
        if (buttonLoader.item && buttonLoader.item.navigation) {
            buttonLoader.item.navigation.requestActive()
        }
    }

    enabled: propertyItem && propertyItem.isEnabled
    visible: propertyItem && propertyItem.isVisible

    width: parent.width

    spacing: 8

    //! NOTE Overridden in instances and specializations of InspectorPropertyView
    function focusOnFirst() {
        if (buttonLoader.item && buttonLoader.item.navigation) {
            buttonLoader.item.navigation.requestActive()
        }
    }

    onRequestResetToDefault: {
        root.propertyItem.resetToDefault()
    }

    onRequestApplyToStyle: {
        root.propertyItem.applyToStyle()
    }

    RowLayout {
        id: contentRow

        width: parent.width
        height: implicitHeight

        spacing: 4

        Loader {
            id: titleLabelLoader

            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true

            visible: Boolean(root.titleText)

            sourceComponent: root.titleLabelComponent ? root.titleLabelComponent : defaultTitleLabel

            Component {
                id: defaultTitleLabel

                StyledTextLabel {
                    width: parent.width
                    visible: !isEmpty

                    text: root.titleText
                    horizontalAlignment: Text.AlignLeft
                }
            }
        }

        Loader {
            id: buttonLoader

            active: visible
            sourceComponent: root.isStyled ? menuButtonComponent : resetButtonComponent
            Layout.alignment: Qt.AlignRight

            Component {
                id: resetButtonComponent

                PropertyResetButton {
                    navigation.name: root.navigationName + "Reset"
                    navigation.panel: root.navigationPanel
                    navigation.row: root.buttonNavigationRow
                    navigation.accessible.name: root.titleText ? qsTrc("inspector", "Reset “%1” to default value").arg(root.titleText)
                                                               : qsTrc("inspector", "Reset property to default value")

                    enabled: root.isModified

                    onClicked: {
                        root.requestResetToDefault()
                    }
                }
            }

            Component {
                id: menuButtonComponent

                MenuButton {
                    width: 20
                    height: width

                    navigation.name: root.navigationName + " Menu Button"
                    navigation.panel: root.navigationPanel
                    navigation.row: root.buttonNavigationRow
                    navigation.accessible.name: root.titleText + " " + qsTrc("inspector", "Menu")

                    menuModel: {
                        var result = []

                        result.push({ title: qsTrc("inspector", "Reset"), enabled: root.isModified, icon: IconCode.UNDO, id: "reset" })

                        if (root.isModified) {
                            result.push({ title: qsTrc("inspector", "Save as default style for this score"), enabled: !root.propertyItem.isUndefined, icon: IconCode.SAVE, id: "save" })
                        } else {
                            result.push({ title: qsTrc("inspector", "This is set as the default style for this score"), enabled: false, icon: IconCode.TICK_RIGHT_ANGLE })
                        }

                        return result
                    }

                    menuAlign: Qt.AlignHCenter

                    onHandleMenuItem: function(itemId) {
                        switch (itemId) {
                        case "reset":
                            root.requestResetToDefault()
                            break
                        case "save":
                            root.requestApplyToStyle()
                            break
                        }
                    }
                }
            }
        }
    }
}
