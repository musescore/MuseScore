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

    property PropertyItem startArrowHeight: root.model ? (root.model.startFilledArrow ? root.model.startFilledArrowHeight : root.model.startLineArrowHeight) : null
    property PropertyItem startArrowWidth: root.model ? (root.model.startFilledArrow ? root.model.startFilledArrowWidth : root.model.startLineArrowWidth) : null
    property PropertyItem endArrowHeight: root.model ? (root.model.endFilledArrow ? root.model.endFilledArrowHeight : root.model.endLineArrowHeight) : null
    property PropertyItem endArrowWidth: root.model ? (root.model.endFilledArrow ? root.model.endFilledArrowWidth : root.model.endLineArrowWidth) : null

    property var possibleStartHookTypes: root.model ? root.model.possibleStartHookTypes : null
    property var possibleEndHookTypes: root.model ? root.model.possibleEndHookTypes: null

    property bool showEndHookRow: root.model && !root.model.showStartHookHeight && root.model.showEndHookHeight

    readonly property bool startHookSectionIsUseful: Boolean(root.possibleStartHookTypes) && root.possibleStartHookTypes.length > 1
    readonly property bool endHookSectionIsUseful: Boolean(root.possibleEndHookTypes) && root.possibleEndHookTypes.length > 1
    readonly property bool hookHeightSectionsIsUseful: (root.startHookHeight && root.startHookHeight.isVisible && root.model.showStartHookHeight) || (root.endHookHeight && root.endHookHeight.isVisible&& root.model.showEndHookHeight)
    readonly property bool startArrowSectionIsUseful: ((root.startArrowHeight && root.startArrowHeight.isVisible) || (root.startArrowWidth && root.startArrowWidth.isVisible)) && root.model && root.model.showStartArrowSettings
    readonly property bool endArrowSectionIsUseful: ((root.endArrowHeight && root.endArrowHeight.isVisible) || (root.endArrowWidth && root.endArrowWidth.isVisible)) && root.model && root.model.showEndArrowSettings
    readonly property bool startHookHeightIsUseful: root.model ? root.model.showStartHookHeight : false
    readonly property bool endHookHeightIsUseful: root.model ? root.model.showEndHookHeight : false
    readonly property bool hookHeightSectionEndIsUseful: root.endHookHeight && root.endHookHeight.isVisible

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    readonly property int navigationRowEnd: endArrowSectionLoader.active ? endArrowSectionLoader.item.navigationRowEnd : (startArrowSectionLoader.active ? startArrowSectionLoader.item.navigationRowEnd : (hookHeightSectionsLoader.active ? hookHeightSectionsLoader.item.navigationRowEnd : hooksRowLoader.item.navigationRowEnd))

    // NOTE can't bind to `visible` property of children, because children are always invisible when parent is invisible
    visible: startHookSectionIsUseful || endHookSectionIsUseful || hookHeightSectionsIsUseful || startArrowSectionIsUseful || endArrowSectionIsUseful

    width: parent.width

    spacing: 0 // Using conditional Layout.bottomMargin on loaders instead

    function getIconCode(possibleHooks, selectedHook) {
        var item = possibleHooks.find(obj => obj.id === Number(selectedHook));
        return item ? item.icon : IconCode.NONE;
    }

    Loader {
        id: hooksRowLoader
        active: startHookSectionIsUseful || endHookSectionIsUseful
        sourceComponent: hooksRowComponent

        Layout.preferredWidth: active ? parent.width : 0
        Layout.preferredHeight: active && item ? item.implicitHeight : 0
        Layout.topMargin: 0
        Layout.bottomMargin: active && (hookHeightSectionsLoader.active || startArrowSectionLoader.active || endArrowSectionLoader.active || hookHeightSectionEndLoader.active) ? 12 : 0
    }

    Component {
        id: hooksRowComponent
        RowLayout {
            width: hooksRowLoader.width
            height: childrenRect.height
            spacing: 0 // Using conditional Layout.leftMargin on loaders instead
            readonly property int navigationRowEnd: startHookSectionLoader.active ? startHookSectionLoader.item.navigationRowEnd : endHookSectionLoader.item.navigationRowEnd

            Loader {
                id: startHookSectionLoader
                active: root.startHookSectionIsUseful
                sourceComponent: startHookSectionComponent

                Layout.preferredWidth: active ? (endHookSectionLoader.active ? (parent.width - parent.spacing) / 2 : parent.width) : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
                Layout.leftMargin: 0
            }

            Component {
                id: startHookSectionComponent
                InspectorPropertyView {
                    id: startHookSection

                    titleText: qsTrc("inspector", "Line start")

                    propertyItem: root.startHookType

                    onRequestResetToDefault: {
                        propertyItem?.resetToDefault?.()
                    }

                    navigationPanel: root.navigationPanel
                    navigationRowStart: root.navigationRowStart
                    navigationRowEnd: startHookDropdown.navigation.row

                    width: startHookSectionLoader.Layout.preferredWidth

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
                                if (!root.startHookType) {
                                    return
                                }

                                root.startHookType.value = itemId;
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
            }

            Loader {
                id: endHookSectionLoader
                active: root.endHookSectionIsUseful
                sourceComponent: endHookSectionComponent

                Layout.preferredWidth: active ? (startHookSectionLoader.active ? (parent.width - parent.spacing) / 2 : parent.width) : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
                Layout.leftMargin: startHookSectionLoader.active ? 8 : 0
            }

            Component {
                id: endHookSectionComponent
                InspectorPropertyView {
                    id: endHookSection

                    titleText: qsTrc("inspector", "Line end")

                    propertyItem: root.endHookType

                    onRequestResetToDefault: {
                        propertyItem?.resetToDefault?.()
                    }

                    navigationPanel: root.navigationPanel
                    navigationRowStart: startHookSectionLoader.active ? startHookSectionLoader.item.navigationRowEnd + 1 : root.navigationRowStart
                    navigationRowEnd: endHookDropdown.navigation.row

                    width: endHookSectionLoader.Layout.preferredWidth

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
                                if (!root.endHookType) {
                                    return
                                }
                                root.endHookType.value = itemId;
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
        }
    }

    Loader {
        id: hookHeightSectionsLoader
        active: hookHeightSectionsIsUseful && !showEndHookRow
        sourceComponent: hookHeightSectionsComponent

        Layout.preferredWidth: active ? parent.width : 0
        Layout.preferredHeight: active && item ? item.implicitHeight : 0
        Layout.bottomMargin: active && (startArrowSectionLoader.active || endArrowSectionLoader.active || hookHeightSectionEndLoader.active) ? 12 : 0
        Layout.topMargin: 0

        readonly property int navigationRowEnd: item ? item.navigationRowEnd : 0
    }

    Component {
        id: hookHeightSectionsComponent
        RowLayout {
            id: hookHeightSections

            readonly property int navigationRowEnd: endHookHeightSectionLoader.active ? endHookHeightSectionLoader.item.navigationRowEnd : startHookHeightSectionLoader.item.navigationRowEnd

            width: hookHeightSectionsLoader.width
            spacing: 8

            Loader {
                id: startHookHeightSectionLoader
                active: root.startHookHeightIsUseful
                sourceComponent: startHookHeightSectionComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: startHookHeightSectionComponent
                SpinBoxPropertyView {
                    id: startHookHeightSection

                    width: startHookHeightSectionLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "Start hook height")
                    propertyItem: root.startHookHeight

                    step: 0.5
                    maxValue: 1000.0
                    minValue: -1000.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "StartHookHeight"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: endHookSectionLoader.active ? endHookSectionLoader.item.navigationRowEnd + 1 : root.navigationRowStart
                }
            }

            Loader {
                id: endHookHeightSectionLoader
                active: root.endHookHeightIsUseful
                sourceComponent: endHookHeightSectionComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: endHookHeightSectionComponent
                SpinBoxPropertyView {
                    id: endHookHeightSection

                    width: endHookHeightSectionLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "End hook height")
                    propertyItem: root.endHookHeight

                    step: 0.5
                    maxValue: 1000.0
                    minValue: -1000.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "EndHookHeight"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: startHookHeightSectionLoader.active ? startHookHeightSectionLoader.item.navigationRowEnd + 1 : (endHookSectionLoader.active ? endHookSectionLoader.item.navigationRowEnd + 1 : root.navigationRowStart)
                }
            }
        }
    }

    Loader {
        id: startArrowSectionLoader
        active: startArrowSectionIsUseful
        sourceComponent: startArrowSectionComponent

        Layout.preferredWidth: active ? parent.width : 0
        Layout.preferredHeight: active && item ? item.implicitHeight : 0
        Layout.topMargin: 0
        Layout.bottomMargin: active && (endArrowSectionLoader.active || hookHeightSectionEndLoader.active) ? 12 : 0
    }

    Component {
        id: startArrowSectionComponent
        RowLayout {
            id: startArrowSection

            readonly property int navigationRowEnd: startArrowWidthLoader.item.navigationRowEnd

            width: startArrowSectionLoader.width
            spacing: 8

            Loader {
                id: startArrowHeightLoader
                active: root.startArrowHeight && root.startArrowHeight.isVisible
                sourceComponent: startArrowHeightComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: startArrowHeightComponent
                SpinBoxPropertyView {
                    id: startArrowHeight

                    width: startArrowHeightLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "Start arrow height")
                    propertyItem: root.startArrowHeight

                    step: 0.1
                    maxValue: 20.0
                    minValue: 0.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "StartArrowHeight"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: (hookHeightSectionsLoader.active ? hookHeightSectionsLoader.item.navigationRowEnd : endHookSectionLoader.item.navigationRowEnd) + 1
                }
            }

            Loader {
                id: startArrowWidthLoader
                active: root.startArrowWidth && root.startArrowWidth.isVisible
                sourceComponent: startArrowWidthComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: startArrowWidthComponent
                SpinBoxPropertyView {
                    id: startArrowWidth

                    width: startArrowWidthLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "Start arrow width")
                    propertyItem: root.startArrowWidth

                    step: 0.1
                    maxValue: 20.0
                    minValue: 0.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "StartArrowWidth"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: startArrowHeightLoader.active ? startArrowHeightLoader.item.navigationRowEnd + 1 : (hookHeightSectionsLoader.active ? hookHeightSectionsLoader.item.navigationRowEnd : endHookSectionLoader.item.navigationRowEnd) + 1
                }
            }
        }
    }

    Loader {
        id: endArrowSectionLoader
        active: endArrowSectionIsUseful
        sourceComponent: endArrowSectionComponent

        Layout.preferredWidth: active ? parent.width : 0
        Layout.preferredHeight: active && item ? item.implicitHeight : 0
        Layout.topMargin: 0
        Layout.bottomMargin: active && hookHeightSectionEndLoader.active ? 12 : 0
    }

    Component {
        id: endArrowSectionComponent
        RowLayout {
            id: endArrowSection

            readonly property int navigationRowEnd: endArrowWidthLoader.item.navigationRowEnd

            width: endArrowSectionLoader.width
            spacing: 8

            Loader {
                id: endArrowHeightLoader
                active: root.endArrowHeight && root.endArrowHeight.isVisible
                sourceComponent: endArrowHeightComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: endArrowHeightComponent
                SpinBoxPropertyView {
                    id: endArrowHeight

                    width: endArrowHeightLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "End arrow height")
                    propertyItem: root.endArrowHeight

                    step: 0.1
                    maxValue: 20.0
                    minValue: 0.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "EndArrowHeight"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: (startArrowSectionLoader.active ? startArrowSectionLoader.item.navigationRowEnd : (hookHeightSectionsLoader.active ? hookHeightSectionsLoader.item.navigationRowEnd : endHookSectionLoader.item.navigationRowEnd)) + 1
                }
            }

            Loader {
                id: endArrowWidthLoader
                active: root.endArrowWidth && root.endArrowWidth.isVisible
                sourceComponent: endArrowWidthComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: endArrowWidthComponent
                SpinBoxPropertyView {
                    id: endArrowWidth

                    width: endArrowWidthLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "End arrow width")
                    propertyItem: root.endArrowWidth

                    step: 0.1
                    maxValue: 20.0
                    minValue: 0.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "EndArrowWidth"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: endArrowHeightLoader.active ? endArrowHeightLoader.item.navigationRowEnd + 1 : (startArrowSectionLoader.active ? startArrowSectionLoader.item.navigationRowEnd : (hookHeightSectionsLoader.active ? hookHeightSectionsLoader.item.navigationRowEnd : endHookSectionLoader.item.navigationRowEnd)) + 1
                }
            }
        }
    }

    Loader {
        id: hookHeightSectionEndLoader
        active: showEndHookRow && hookHeightSectionEndIsUseful
        sourceComponent: hookHeightSectionEndComponent

        Layout.preferredWidth: active ? parent.width : 0
        Layout.preferredHeight: active && item ? item.implicitHeight : 0
        Layout.topMargin: 0
        Layout.bottomMargin: 0 // Last item, no bottom margin
    }

    Component {
        id: hookHeightSectionEndComponent
        RowLayout {
            id: hookHeightSectionEnd

            spacing: 8
            width: hookHeightSectionEndLoader.width

            Loader {
                id: endHookHeightSectionEndLoader
                active: root.hookHeightSectionEndIsUseful
                sourceComponent: endHookHeightSectionEndComponent

                Layout.preferredWidth: active ? (parent.width - parent.spacing) / 2 : 0
                Layout.preferredHeight: active && item ? item.implicitHeight : 0
            }

            Component {
                id: endHookHeightSectionEndComponent
                SpinBoxPropertyView {
                    id: endHookHeightSectionEnd

                    width: endHookHeightSectionEndLoader.Layout.preferredWidth

                    titleText: qsTrc("inspector", "End hook height")
                    propertyItem: root.endHookHeight

                    step: 0.5
                    maxValue: 1000.0
                    minValue: -1000.0
                    decimals: 2
                    measureUnitsSymbol: qsTrc("global", "sp")

                    navigationName: "EndHookHeight"
                    navigationPanel: root.navigationPanel
                    navigationRowStart: (endArrowSectionLoader.active ? endArrowSectionLoader.item.navigationRowEnd :
                                        (startArrowSectionLoader.active ? startArrowSectionLoader.item.navigationRowEnd :
                                         hooksRowLoader.item.navigationRowEnd)) + 1
                }
            }
        }
    }
}
