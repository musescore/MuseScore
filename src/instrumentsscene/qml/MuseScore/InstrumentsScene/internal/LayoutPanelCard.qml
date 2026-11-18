/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.InstrumentsScene

FocusableControl {
    id: root

    required property AbstractLayoutPanelTreeItem item
    required property TreeView treeView
    required property var modelIndex
    required property int row
    required property int depth
    required property bool isExpanded
    property bool isDragged: false

    property bool isInCombinedGroup: false

    readonly property int type: item ? item.type : LayoutPanelItemType.UNDEFINED
    readonly property bool isSelectable: item && item.isSelectable
    readonly property bool isExpandable: item && item.isExpandable

    property int sideMargin: 0
    property int cardRadius: 0

    readonly property alias isPopupOpened: cardContent.isPopupOpened

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal removeSelectionRequested()

    signal changeVisibilityOfSelectedRowsRequested(bool visible)
    signal changeVisibilityRequested(var modelIndex, bool visible)
    signal changeEnabledOfSelectedRowsRequested(bool enable)
    signal changeEnabledRequested(var modelIndex, bool enable)

    signal expandRequested(bool expand)

    signal mousePressed()
    signal mouseReleased()

    QtObject {
        id: prv

        readonly property bool isEnabled: Boolean(root.item?.isEnabled)
        readonly property bool isSelected: Boolean(root.item?.isSelected)
        readonly property bool isPressed: root.mouseArea.pressed || root.isDragged

        // A group header's state is drawn on the whole group's background
        readonly property bool usesRowHighlight: root.type !== LayoutPanelItemType.SHARED_PART

        readonly property bool isExpandedPart: root.isExpanded && root.type === LayoutPanelItemType.PART

        readonly property bool isCardTop: root.depth === 0 || root.isDragged
        readonly property bool isCardBottom: (root.depth === 0 && !prv.isExpandedPart) || root.isDragged
        readonly property bool isInExpandedCard: root.depth > 0 || prv.isExpandedPart
    }

    implicitHeight: 30
    implicitWidth: 248

    navigation.name: "LayoutPanelCard"
    navigation.column: 0

    navigation.accessible.role: MUAccessible.ListItem
    navigation.accessible.name: cardContent.title

    onNavigationTriggered: { root.clicked(null) }

    mouseArea.propagateComposedEvents: true

    mouseArea.hoverEnabled: root.visible

    mouseArea.onClicked: function(mouse) { root.clicked(mouse) }
    mouseArea.onDoubleClicked: function(mouse) { root.doubleClicked(mouse) }
    mouseArea.onPressed: { root.mousePressed() }
    mouseArea.onReleased: { root.mouseReleased() }
    mouseArea.onCanceled: { root.mouseReleased() }
    mouseArea.enabled: root.isSelectable

    background.topLeftRadius: prv.isCardTop ? root.cardRadius : 0
    background.topRightRadius: prv.isCardTop ? root.cardRadius : 0
    background.bottomLeftRadius: prv.isCardBottom ? root.cardRadius : 0
    background.bottomRightRadius: prv.isCardBottom ? root.cardRadius : 0

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

    StyledRectangularShadow {
        id: shadow

        anchors.fill: root.background
        visible: false
        z: -1
    }

    LayoutPanelCardContent {
        id: cardContent

        anchors.fill: parent
        anchors.leftMargin: root.sideMargin
        anchors.rightMargin: root.sideMargin

        item: root.item
        isStaveSharingEnabled: root.treeView.model.isStaveSharingEnabled
        isHovered: root.mouseArea.containsMouse
        isHighlighted: prv.isSelected || prv.isPressed || root.navigation.highlight

        navigationPanel: root.navigation.panel
        navigationRow: root.navigation.row

        depth: root.depth
        isExpanded: root.isExpanded
        isEnabled: prv.isEnabled
        isInCombinedGroup: root.isInCombinedGroup

        onVisibilityToggled: function(visible) {
            if (prv.isSelected) {
                root.changeVisibilityOfSelectedRowsRequested(visible)
            } else {
                root.changeVisibilityRequested(root.modelIndex, visible)
            }
        }

        onExpandToggled: function(expand) {
            root.expandRequested(expand)
        }

        onEnabledToggled: function(enabled) {
            if (prv.isSelected) {
                root.changeEnabledOfSelectedRowsRequested(enabled)
            } else {
                root.changeEnabledRequested(root.modelIndex, enabled)
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
            when: prv.usesRowHighlight && root.mouseArea.containsMouse && !prv.isPressed && !prv.isSelected

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: prv.usesRowHighlight && root.mouseArea.pressed && !prv.isSelected && !root.isDragged

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: prv.usesRowHighlight && prv.isSelected && !root.mouseArea.containsMouse && !prv.isPressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: prv.usesRowHighlight && prv.isSelected && root.mouseArea.containsMouse && !prv.isPressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
            }
        },

        State {
            name: "SELECTED_PRESSED"
            when: prv.usesRowHighlight && prv.isSelected && prv.isPressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
            }
        },

        State {
            name: "IN_EXPANDED_CARD"
            when: prv.isInExpandedCard && !prv.isSelected

            PropertyChanges {
                target: root.background
                color: ui.theme.textFieldColor
                opacity: 1
            }
        }
    ]

    states: [
        State {
            when: root.isDragged
            name: "DRAGGED"

            PropertyChanges {
                target: shadow
                visible: true
            }

            PropertyChanges {
                target: root
                opacity: 0.9
            }
        }
    ]
}
