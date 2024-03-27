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
import QtQuick
import QtQuick.Layouts

import MuseScore.Ui
import MuseScore.UiComponents
import MuseScore.InstrumentsScene

FocusableControl {
    id: root

    property TreeView treeView: null

    required property QtObject item
    required property int row
    required property var modelIndex
    required property bool isExpanded
    required property int depth

    property string filterKey

    readonly property int type: item ? item.type : InstrumentsTreeItemType.UNDEFINED
    readonly property bool isSelected: item && item.isSelected
    readonly property bool isDragAvailable: item && item.isSelectable
    readonly property bool isExpandable: item && item.isExpandable
    readonly property bool isEditable: item && item.isEditable

    property int sideMargin: 0

    property var popupAnchorItem: null

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal toggleExpandedRequested()
    signal removeSelectionRequested()

    signal popupOpened(var popupX, var popupY, var popupHeight)
    signal popupClosed()

    signal visibilityChanged(bool visible)

    signal dragStarted()
    signal dropped()

    onDoubleClicked: {
        toggleExpandedRequested()
    }

    QtObject {
        id: prv

        property bool dragged: mouseArea.drag.active && mouseArea.pressed

        onDraggedChanged: {
            if (dragged) {
                root.dragStarted()
                if (root.isExpanded) {
                    root.treeView.collapse(root.row)
                }
            } else {
                root.dropped()
            }
        }

        property var openedPopup: null
        property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(popup, data) {
            if (Boolean(popup)) {
                openedPopup = popup
                popup.load(data)
                root.popupOpened(popup.x, popup.y, popup.height)
                popup.open()
            }
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                openedPopup.close()
                resetOpenedPopup()
            }
        }

        function resetOpenedPopup() {
            root.popupClosed()
            openedPopup = null
        }
    }

    anchors.fill: parent

    implicitHeight: 38
    implicitWidth: 248

    Drag.keys: [ root.filterKey ]
    Drag.active: prv.dragged && isDragAvailable
    Drag.source: root
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    navigation.name: "InstrumentsTreeItemDelegate"
    navigation.column: 0
    navigation.row: root.row

    navigation.accessible.role: MUAccessible.ListItem
    navigation.accessible.name: titleLabel.text

    onNavigationTriggered: { root.clicked(null) }

    mouseArea.preventStealing: true
    mouseArea.propagateComposedEvents: true

    mouseArea.hoverEnabled: root.visible

    mouseArea.onClicked: function(mouse) { root.clicked(mouse) }
    mouseArea.onDoubleClicked: function(mouse) { root.doubleClicked(mouse) }

    mouseArea.drag.target: root
    mouseArea.drag.axis: Drag.YAxis

    Keys.onShortcutOverride: function(event) {
        switch (event.key) {
        case Qt.Key_Backspace:
        case Qt.Key_Delete:
            event.accepted = true
            root.removeSelectionRequested()
            break
        default:
            break
        }
    }

    StyledDropShadow {
        id: shadow

        anchors.fill: parent
        source: background
        visible: false
    }

    Loader {
        id: popupLoader

        function createPopup(comp, btn) {
            popupLoader.sourceComponent = comp
            popupLoader.item.parent = btn
            popupLoader.item.needActiveFirstItem = btn.navigation.highlight
            return popupLoader.item
        }
    }

    Component {
        id: instrumentSettingsComp

        InstrumentSettingsPopup {
            anchorItem: popupAnchorItem

            onClosed: {
                prv.resetOpenedPopup()
                popupLoader.sourceComponent = null
            }
        }
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {
            anchorItem: popupAnchorItem

            onClosed: {
                prv.resetOpenedPopup()
                popupLoader.sourceComponent = null
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root.sideMargin
        anchors.rightMargin: root.sideMargin

        spacing: 2

        VisibilityBox {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: width

            objectName: "VisibleBtnInstrument"
            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 1

            isVisible: root.item && root.item.isVisible

            onVisibleToggled: {
                if (!root.item) {
                    return
                }

                if (root.isSelected) {
                    root.visibilityChanged(!isVisible)
                } else {
                    root.item.isVisible = !isVisible
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 12 * root.depth
            height: childrenRect.height

            FlatButton {
                id: expandButton
                anchors.left: parent.left

                visible: root.isExpandable

                objectName: "ExpandBtnInstrument"
                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 2
                navigation.accessible.name: root.isExpanded
                                            //: Collapse a tree item
                                            ? qsTrc("global", "Collapse")
                                            //: Expand a tree item
                                            : qsTrc("global", "Expand")

                transparent: true
                icon: root.isExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

                onClicked: {
                    root.toggleExpandedRequested()
                }
            }

            StyledTextLabel {
                id: titleLabel

                anchors.left: expandButton.right
                anchors.leftMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: expandButton.verticalCenter

                text: root.item ? root.item.title : ""
                horizontalAlignment: Text.AlignLeft
                opacity: root.item && root.item.isVisible ? 1 : 0.75

                font: {
                    if (root.item && root.type === InstrumentsTreeItemType.PART && root.item.isVisible) {
                        return ui.theme.bodyBoldFont
                    }

                    return ui.theme.bodyFont
                }
            }
        }

        FlatButton {
            id: settingsButton

            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: width

            visible: root.isEditable

            objectName: "SettingsBtnInstrument"
            enabled: root.visible
            navigation.panel: root.navigation.panel
            navigation.row: root.navigation.row
            navigation.column: 3
            navigation.accessible.name: qsTrc("instruments", "Settings")

            icon: IconCode.SETTINGS_COG

            onClicked: {
                if (prv.isPopupOpened) {
                    prv.closeOpenedPopup()
                    return
                }

                var popup = null
                var data = {}

                if (root.type === InstrumentsTreeItemType.PART) {
                    popup = popupLoader.createPopup(instrumentSettingsComp, this)

                    data["partId"] = root.item.id
                    data["instrumentId"] = root.item.instrumentId()
                } else if (root.type === InstrumentsTreeItemType.STAFF) {
                    popup = popupLoader.createPopup(staffSettingsComp, this)

                    data["id"] = root.item.id
                }

                prv.openPopup(popup, data)
            }

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            opacity = 1.0
        } else {
            opacity = 0.0
        }
    }

    Behavior on opacity {
        enabled: root.depth !== 0
        NumberAnimation { duration: 150 }
    }

    focusBorder.drawOutsideParent: false

    background.states: [
        State {
            name: "HOVERED"
            when: root.mouseArea.containsMouse && !root.mouseArea.pressed && !root.isSelected && !prv.dragged

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.mouseArea.pressed && !root.isSelected && !prv.dragged

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.isSelected && !root.mouseArea.containsMouse && !root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: root.isSelected && root.mouseArea.containsMouse && !root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
            }
        },

        State {
            name: "SELECTED_PRESSED"
            when: root.isSelected && root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
            }
        },

        State {
            name: "PART_EXPANDED"
            when: root.isExpanded && !root.isSelected &&
                  root.type === InstrumentsTreeItemType.PART

            PropertyChanges {
                target: root.background
                color: ui.theme.textFieldColor
                opacity: 1
            }
        },

        State {
            name: "PARENT_EXPANDED"
            when: root.visible && !root.isSelected &&
                  (root.type === InstrumentsTreeItemType.INSTRUMENT ||
                   root.type === InstrumentsTreeItemType.STAFF)

            PropertyChanges {
                target: root.background
                color: ui.theme.textFieldColor
                opacity: 1
            }
        }
    ]

    states: [
        State {
            when: prv.dragged
            name: "DRAGGED"

            ParentChange {
                target: root
                parent: root.treeView.contentItem
            }

            PropertyChanges {
                target: shadow
                visible: true
            }

            PropertyChanges {
                target: root
                height: implicitHeight
                width: root.treeView.contentItem.width
            }

            AnchorChanges {
                target: root
                anchors {
                    verticalCenter: undefined
                    horizontalCenter: undefined
                }
            }
        }
    ]
}
