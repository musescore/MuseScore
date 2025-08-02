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
import MuseScore.InstrumentsScene 1.0

FocusableControl {
    id: root

    required property var originalParent

    property var item: null
    property LegacyTreeView treeView: undefined
    property var index: styleData.index
    property string filterKey

    readonly property int type: item ? item.type : LayoutPanelItemType.UNDEFINED
    readonly property bool isSelected: item && item.isSelected
    readonly property bool isSelectable: item && item.isSelectable
    readonly property bool isExpandable: item && item.isExpandable
    readonly property bool settingsAvailable: item && item.settingsAvailable
    readonly property bool settingsEnabled: item && item.settingsEnabled

    property int sideMargin: 0

    property var popupAnchorItem: null

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal removeSelectionRequested()

    signal popupOpened(var popupX, var popupY, var popupHeight)
    signal popupClosed()

    signal changeVisibilityOfSelectedRowsRequested(bool visible)
    signal changeVisibilityRequested(var index, bool visible)

    signal dragStarted()
    signal dropped()

    QtObject {
        id: prv

        property bool dragged: mouseArea.drag.active && mouseArea.pressed

        onDraggedChanged: {
            if (dragged) {
                root.dragStarted()
                if (styleData.isExpanded) {
                    root.treeView.collapse(styleData.index)
                }
            } else {
                root.dropped()
            }
        }
    }

    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    implicitHeight: 38
    implicitWidth: 248

    Drag.keys: [ root.filterKey ]
    Drag.active: prv.dragged && root.isSelectable
    Drag.source: root
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    navigation.name: "LayoutPanelItemDelegate"
    navigation.column: 0

    navigation.accessible.role: MUAccessible.ListItem
    navigation.accessible.name: visibilityControls.title

    onNavigationTriggered: { root.clicked(null) }

    mouseArea.preventStealing: true
    mouseArea.propagateComposedEvents: true

    mouseArea.hoverEnabled: root.visible

    mouseArea.onClicked: function(mouse) { root.clicked(mouse) }
    mouseArea.onDoubleClicked: function(mouse) { root.doubleClicked(mouse) }
    mouseArea.enabled: root.isSelectable

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

        readonly property StyledPopupView openedPopup: popupLoader.item as StyledPopupView
        readonly property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(comp: Component, btn: Item, item) {
            popupLoader.sourceComponent = comp
            if (!openedPopup) {
                return
            }

            openedPopup.parent = btn
            openedPopup.needActiveFirstItem = btn.navigation.highlight

            openedPopup.load(item)

            openedPopup.opened.connect(function() {
                root.popupOpened(openedPopup.x, openedPopup.y, openedPopup.height)
            })

            openedPopup.closed.connect(function() {
                root.popupClosed()
                sourceComponent = null
            })

            openedPopup.open()
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                openedPopup.close()
            }
        }
    }

    Component {
        id: instrumentSettingsComp

        InstrumentSettingsPopup {
            anchorItem: popupAnchorItem

            onReplaceInstrumentRequested: {
                // The popup would close when the dialog to select the new
                // instrument is shown; when it closes, it is unloaded, i.e.
                // deleted, which means that it is deleted while a signal
                // handler inside it is being executed. This causes a crash.
                // To prevent that, let the popup close itself, and perform the
                // actual operation "later", i.e. not (directly or indirectly)
                // inside the signal handler in the popup.
                Qt.callLater(model.itemRole.replaceInstrument)
            }

            onResetAllFormattingRequested: {
                // Same as above
                Qt.callLater(model.itemRole.resetAllFormatting)
            }
        }
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {
            anchorItem: popupAnchorItem
        }
    }

    Component {
        id: systemObjectsLayerSettingsComp

        SystemObjectsLayerSettingsPopup {
            anchorItem: popupAnchorItem
        }
    }

    VisibilityControls {
        id: visibilityControls

        anchors.fill: parent
        anchors.leftMargin: root.sideMargin
        anchors.rightMargin: root.sideMargin

        navigationPanel: root.navigation.panel
        navigationRow: root.navigation.row

        title: Boolean(model) ? model.itemRole.title : ""
        isRootControl: Boolean(model) && root.type === LayoutPanelItemType.PART

        useVisibilityButton: root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER
        isVisible: Boolean(model) && model.itemRole.isVisible
        onVisibilityButtonClicked: function(isVisible) {
            if (!model) {
                return
            }
            if (root.isSelected) {
                root.changeVisibilityOfSelectedRowsRequested(!isVisible)
            } else {
                root.changeVisibilityRequested(styleData.index, !isVisible)
            }
        }

        showDashIcon: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER

        isExpandable: root.isExpandable
        isExpanded: styleData.isExpanded
        expandableDepth: styleData.depth
        onExpandButtonClicked: function(expand) {
            if (expand) {
                root.treeView.expand(styleData.index)
            } else {
                root.treeView.collapse(styleData.index)
            }
        }

        rightSideButtonComp: FlatButton {
            id: settingsButton

            visible: root.settingsAvailable
            enabled: root.visible && root.settingsEnabled

            objectName: "SettingsBtn"
            navigation.panel: visibilityControls.navigationPanel
            navigation.row: visibilityControls.navigationRow
            navigation.column: 3
            navigation.accessible.name: qsTrc("layoutpanel", "Settings")

            icon: IconCode.SETTINGS_COG

            onClicked: {
                if (popupLoader.isPopupOpened) {
                    popupLoader.closeOpenedPopup()
                    return
                }

                let comp = null
                let item = {}

                if (root.type === LayoutPanelItemType.PART) {
                    comp = instrumentSettingsComp

                    item["partId"] = model.itemRole.id
                    item["instrumentId"] = model.itemRole.instrumentId()
                } else if (root.type === LayoutPanelItemType.STAFF) {
                    comp = staffSettingsComp

                    item["id"] = model.itemRole.id
                } else if (root.type == LayoutPanelItemType.SYSTEM_OBJECTS_LAYER) {
                    comp = systemObjectsLayerSettingsComp

                    item["staffId"] = model.itemRole.staffId()
                }

                popupLoader.openPopup(comp, this, item)
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
        enabled: styleData.depth !== 0
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
            when: styleData.isExpanded && !root.isSelected &&
                  root.type === LayoutPanelItemType.PART

            PropertyChanges {
                target: root.background
                color: ui.theme.textFieldColor
                opacity: 1
            }
        },

        State {
            name: "PARENT_EXPANDED"
            when: root.visible && !root.isSelected &&
                  (root.type === LayoutPanelItemType.INSTRUMENT ||
                   root.type === LayoutPanelItemType.STAFF)

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
                width: treeView.contentItem.width
            }

            AnchorChanges {
                target: root
                anchors {
                    verticalCenter: undefined
                    horizontalCenter: undefined
                }
            }
        },

        //! NOTE: Workaround for a bug in Qt 6.2.4 - see PR #24106 comment
        // https://bugreports.qt.io/browse/QTBUG-99436
        State {
            when: !prv.dragged
            name: "DROPPED"

            ParentChange {
                target: root
                parent: root.originalParent
            }
        }

    ]
}
