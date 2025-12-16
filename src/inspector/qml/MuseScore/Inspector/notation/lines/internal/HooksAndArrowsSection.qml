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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../../common"

ColumnLayout {
    id: root

    required property TextLineSettingsModel model

    property PropertyItem startHookType: root.model ? root.model.startHookType : null
    property PropertyItem endHookType: root.model ? root.model.endHookType : null
    property PropertyItem startHookHeight: root.model ? root.model.startHookHeight : null
    property PropertyItem endHookHeight: root.model ? root.model.endHookHeight : null

    property PropertyItem startArrowHeight: root.model ? root.model.startArrowHeight : null
    property PropertyItem startArrowWidth: root.model ? root.model.startArrowWidth : null
    property PropertyItem endArrowHeight: root.model ? root.model.endArrowHeight : null
    property PropertyItem endArrowWidth: root.model ? root.model.endArrowWidth : null

    property var possibleStartHookTypes: root.model ? root.model.possibleStartHookTypes : null
    property var possibleEndHookTypes: root.model ? root.model.possibleEndHookTypes: null

    property bool showEndHookRow: root.model && !root.model.showStartHookHeight && root.model.showEndHookHeight

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    readonly property int navigationRowEnd: endArrowSection.isUseful ? endArrowSection.navigationRowEnd : (startArrowSection.isUseful ? startArrowSection.navigationRowEnd : (hookHeightSections.isUseful ? hookHeightSections.navigationRowEnd : endHookSection.navigationRowEnd))

    // NOTE can't bind to `visible` property of children, because children are always invisible when parent is invisible
    visible: startHookSection.isUseful || endHookSection.isUseful || hookHeightSections.isUseful || startArrowSection.isUseful || endArrowSection.isUseful

    width: parent.width

    spacing: 12

    function getIconCode(possibleHooks, selectedHook) {
        var item = possibleHooks.find(obj => obj.id === Number(selectedHook));
        return item ? item.icon : IconCode.NONE;
    }

    RowLayout {
        width: parent.width
        height: childrenRect.height
        spacing: 8

        InspectorPropertyView {
            id: startHookSection

            titleText: qsTrc("inspector", "Line start")

            propertyItem: root.startHookType

            onRequestResetToDefault: {
                propertyItem?.resetToDefault?.()
            }

            readonly property bool isUseful: Boolean(root.possibleStartHookTypes) && root.possibleStartHookTypes.length > 1
            visible: isUseful

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
            navigationRowEnd: startHookDropdown.navigation.row

            Layout.minimumWidth: (parent.width - parent.spacing) / 2

            FlatButton {
                id: startHookDropdown

                width: parent.width
                icon: getIconCode(root.possibleStartHookTypes, root.startHookType?.value)

                navigation.panel: root.navigationPanel
                navigation.row: startHookSection.navigationRowStart + 1

                mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: startHooksMenu.toggleOpened(startHooksMenu.model)

                StyledIconLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 8

                    iconCode: IconCode.SMALL_ARROW_DOWN
                }

                StyledMenuLoader {
                    id: startHooksMenu

                    property var model: root.possibleStartHookTypes

                    onHandleMenuItem: function(itemId) {
                        if (!startHookType) {
                            return
                        }

                        startHookType.value = itemId;
                    }

                    onOpened: function() {
                        startHookDropdown.accentButton = true;
                    }
                    onClosed: function() {
                        startHookDropdown.accentButton = false;
                    }
                }
            }
        }

        InspectorPropertyView {
            id: endHookSection

            titleText: qsTrc("inspector", "Line end")

            propertyItem: root.endHookType

            onRequestResetToDefault: {
                propertyItem?.resetToDefault?.()
            }

            readonly property bool isUseful: Boolean(root.possibleEndHookTypes) && root.possibleEndHookTypes.length > 1
            visible: isUseful

            navigationPanel: root.navigationPanel
            navigationRowStart: startHookSection.navigationRowEnd + 1
            navigationRowEnd: endHookDropdown.navigation.row

            Layout.minimumWidth: (parent.width - parent.spacing) / 2

            FlatButton {
                id: endHookDropdown

                width: parent.width
                icon: getIconCode(root.possibleEndHookTypes, root.endHookType?.value)

                navigation.panel: root.navigationPanel
                navigation.row: endHookSection.navigationRowStart + 1

                mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: endHooksMenu.toggleOpened(endHooksMenu.model)

                StyledIconLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 8

                    iconCode: IconCode.SMALL_ARROW_DOWN
                }

                StyledMenuLoader {
                    id: endHooksMenu

                    property var model: root.possibleEndHookTypes

                    onHandleMenuItem: function(itemId) {
                        if (!endHookType) {
                            return
                        }
                        endHookType.value = itemId;
                    }

                    onOpened: function() {
                        endHookDropdown.accentButton = true;
                    }
                    onClosed: function() {
                        endHookDropdown.accentButton = false;
                    }
                }
            }
        }
    }

    RowLayout {
        id: hookHeightSections

        readonly property bool isUseful: (root.startHookHeight && root.startHookHeight.isVisible)
                                         || (root.endHookHeight && root.endHookHeight.isVisible)

        readonly property int navigationRowEnd: (root.endHookHeight && root.endHookHeight.isVisible) ? endHookHeightSection.navigationRowEnd : startHookHeightSection.navigationRowEnd

        // Hide this row when only endHookHeight is visible
        visible: isUseful && !showEndHookRow

        width: parent.width
        spacing: 8

        SpinBoxPropertyView {
            id: startHookHeightSection
            Layout.preferredWidth: (parent.width - parent.spacing) / 2

            visible: root.model ? root.model.showStartHookHeight : false

            titleText: qsTrc("inspector", "Start hook height")
            propertyItem: root.startHookHeight

            step: 0.5
            maxValue: 1000.0
            minValue: -1000.0
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            navigationName: "StartHookHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: endHookSection.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: endHookHeightSection
            Layout.preferredWidth: (parent.width - parent.spacing) / 2

            visible: root.model ? root.model.showEndHookHeight : false

            titleText: qsTrc("inspector", "End hook height")
            propertyItem: root.endHookHeight

            step: 0.5
            maxValue: 1000.0
            minValue: -1000.0
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            navigationName: "EndHookHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: startHookHeightSection.navigationRowEnd + 1
        }
    }

    RowLayout {
        id: startArrowSection

        readonly property bool isUseful: ((root.startArrowHeight && root.startArrowHeight.isVisible)
                                         || (root.startArrowWidth && root.startArrowWidth.isVisible)) && root.model && root.model.showStartArrowSettings

        readonly property int navigationRowEnd: startArrowWidth.navigationRowEnd

        visible: isUseful

        width: parent.width
        spacing: 8

        SpinBoxPropertyView {
            id: startArrowHeight
            Layout.preferredWidth: (parent.width - parent.spacing) / 2

            titleText: qsTrc("inspector", "Start arrow height")
            propertyItem: root.startArrowHeight

            step: 0.1
            maxValue: 20.0
            minValue: 0.0
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            navigationName: "StartArrowHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: (hookHeightSections.isUseful ? hookHeightSections.navigationRowEnd : endHookSection.navigationRowEnd) + 1
        }

        SpinBoxPropertyView {
            id: startArrowWidth
            Layout.preferredWidth: (parent.width - parent.spacing) / 2

            titleText: qsTrc("inspector", "Start arrow width")
            propertyItem: root.startArrowWidth

            step: 0.1
            maxValue: 20.0
            minValue: 0.0
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            navigationName: "StartArrowWidth"
            navigationPanel: root.navigationPanel
            navigationRowStart: startArrowHeight.navigationRowEnd + 1
        }
    }

    RowLayout {
        id: endArrowSection

        readonly property bool isUseful: ((root.endArrowHeight && root.endArrowHeight.isVisible)
                                         || (root.endArrowWidth && root.endArrowWidth.isVisible)) && root.model && root.model.showEndArrowSettings

        readonly property int navigationRowEnd: endArrowWidth.navigationRowEnd

        visible: isUseful

        width: parent.width
        spacing: 8

        SpinBoxPropertyView {
            id: endArrowHeight
            Layout.preferredWidth: (parent.width - parent.spacing) / 2

            titleText: qsTrc("inspector", "End arrow height")
            propertyItem: root.endArrowHeight

            step: 0.1
            maxValue: 20.0
            minValue: 0.0
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            navigationName: "EndArrowHeight"
            navigationPanel: root.navigationPanel
            navigationRowStart: (startArrowSection.isUseful ? startArrowSection.navigationRowEnd : (hookHeightSections.isUseful ? hookHeightSections.navigationRowEnd : endHookSection.navigationRowEnd)) + 1
        }

        SpinBoxPropertyView {
            id: endArrowWidth
            Layout.preferredWidth: (parent.width - parent.spacing) / 2

            titleText: qsTrc("inspector", "End arrow width")
            propertyItem: root.endArrowWidth

            step: 0.1
            maxValue: 20.0
            minValue: 0.0
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            navigationName: "EndArrowWidth"
            navigationPanel: root.navigationPanel
            navigationRowStart: endArrowHeight.navigationRowEnd + 1
        }
    }

    Loader {
        id: hookHeightSectionEndLoader
        width: parent.width
        active: showEndHookRow
        sourceComponent: hookHeightSectionEndComponent

        Layout.preferredWidth: parent.width
        Layout.minimumWidth: parent.width
        Layout.preferredHeight: item ? item.implicitHeight : 0
        Layout.minimumHeight: item ? item.implicitHeight : 0
    }

    Component {
        id: hookHeightSectionEndComponent
        RowLayout {
            id: hookHeightSectionEnd

            readonly property bool isUseful: root.endHookHeight && root.endHookHeight.isVisible

            implicitHeight: childrenRect.height
            spacing: 8
            width: hookHeightSectionEndLoader.width

            SpinBoxPropertyView {
                id: endHookHeightSectionEnd
                Layout.preferredWidth: (parent.width - parent.spacing) / 2

                titleText: qsTrc("inspector", "End hook height")
                propertyItem: root.endHookHeight

                step: 0.5
                maxValue: 1000.0
                minValue: -1000.0
                decimals: 2
                measureUnitsSymbol: qsTrc("global", "sp")

                navigationName: "EndHookHeight"
                navigationPanel: root.navigationPanel
                navigationRowStart: (endArrowSection.isUseful ? endArrowSection.navigationRowEnd :
                                    (startArrowSection.isUseful ? startArrowSection.navigationRowEnd :
                                     endHookSection.navigationRowEnd)) + 1
            }
        }
    }
}
