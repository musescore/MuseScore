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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import Muse.UiComponents.LegacyTreeView

import MuseScore.InstrumentsScene

FocusableControl {
    id: root

    required property AbstractLayoutPanelTreeItem item
    required property LegacyTreeView treeView
    required property var modelIndex
    required property int depth
    required property bool isExpanded
    property string filterKey

    property bool isEnabled: item && item.isEnabled

    readonly property int type: item ? item.type : LayoutPanelItemType.UNDEFINED
    readonly property bool isSelected: item && item.isSelected
    readonly property bool isSelectable: item && item.isSelectable
    readonly property bool isExpandable: item && item.isExpandable
    readonly property bool settingsAvailable: item && item.settingsAvailable
    readonly property bool settingsEnabled: item && item.settingsEnabled

    // Vertical position of the row content within the row (the rest is group padding / spacing, see LayoutPanel.qml)
    property int contentTopMargin: 0
    property int contentHeight: 30

    // Whether this row closes its card (a part together with its expanded staves)
    property bool cardBottom: true

    property bool isInGroup: false
    property bool isGroupCombined: false
    property bool isGroupExpanded: false
    property bool isStaveSharingEnabled: false

    property bool isLastRow: false

    // For staff rows: whether their instrument is visible
    property bool isParentVisible: true

    // The first system markings row can't be moved, so it has no hover / pressed highlight
    // (its cog keeps its own states)
    property bool isLocked: false

    // Versions > Hide disabled eye icons: instruments and staves in combined groups don't show their (disabled) eye;
    // its cell is kept, so nothing shifts
    property bool hideDisabledEyes: false

    // Versions > Only show settings on hover: when off, the cog is always shown
    property bool onlyShowSettingsOnHover: true

    // Shifts the row's contents right, keeping its width (see Versions > Indentation in LayoutPanel.qml)
    property int contentIndent: 0

    property alias isPopupOpened: popupLoader.isPopupOpened

    readonly property bool isCardTop: root.depth === 0
    readonly property bool isInExpandedCard: root.depth > 0 || (root.isExpanded && root.type === LayoutPanelItemType.PART)
    readonly property bool isHovered: rowHover.hovered && !prv.dragged
    readonly property bool isDragged: prv.dragged

    // Group headers show their hover / pressed / selected state on the whole group's background (see LayoutPanel.qml)
    readonly property bool isEmptyGroup: root.type === LayoutPanelItemType.EMPTY_SHARED_GROUP
    readonly property bool usesRowHighlight: root.type !== LayoutPanelItemType.SHARED_PART && !root.isEmptyGroup
    readonly property int cardRadius: 3

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal removeSelectionRequested()
    signal groupExpandToggled()
    signal ungroupRequested()
    // Right-click; x, y in this item's coordinates
    signal contextMenuRequested(real x, real y)
    signal deleteEmptyGroupRequested()

    signal changeVisibilityOfSelectedRowsRequested(bool visible)
    signal changeVisibilityRequested(var modelIndex, bool visible)
    signal changeEnabledOfSelectedRowsRequested(bool enable)
    signal changeEnabledRequested(var modelIndex, bool enable)

    signal dragStarted()
    signal dropped()

    QtObject {
        id: prv

        property bool dragged: root.mouseArea.drag.active && root.mouseArea.pressed

        onDraggedChanged: {
            if (dragged) {
                root.dragStarted()
                if (root.isExpanded) {
                    root.treeView.collapse(root.modelIndex)
                }
            } else {
                root.dropped()

                // On release, the DRAGGED state's ParentChange restores the size the row had before the drag as fixed
                // values, replacing the bindings to the row's slot below. As the row may have moved to a slot of a
                // different height (e.g. becoming the last instrument of a group), bring the bindings back.
                Qt.callLater(function() {
                    if (!root || prv.dragged) {
                        return
                    }
                    root.width = Qt.binding(function() { return root.slot ? root.slot.width : root.implicitWidth })
                    root.height = Qt.binding(function() { return root.slot ? root.slot.height : root.implicitHeight })
                })
            }
        }
    }

    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

    // The row's slot in the list (its original parent). While dragged, the row is reparented to the view's content
    // item, so its size is bound to the slot rather than to whatever its parent currently is: otherwise the dragged
    // row would take the whole list's size (and the drag's hot spot would be far off)
    property Item slot: null
    Component.onCompleted: root.slot = root.parent

    height: slot ? slot.height : (parent ? parent.height : implicitHeight)
    width: slot ? slot.width : (parent ? parent.width : implicitWidth)

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
    navigation.accessible.name: titleLabel.text

    onNavigationTriggered: { root.clicked(null) }

    mouseArea.anchors.fill: contentArea
    mouseArea.preventStealing: true
    mouseArea.propagateComposedEvents: true

    mouseArea.hoverEnabled: root.visible

    mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton
    mouseArea.onClicked: function(mouse) {
        if (mouse.button === Qt.RightButton) {
            const pos = root.mouseArea.mapToItem(root, mouse.x, mouse.y)
            root.contextMenuRequested(pos.x, pos.y)
            return
        }
        root.clicked(mouse)
    }
    mouseArea.onDoubleClicked: function(mouse) { root.doubleClicked(mouse) }
    mouseArea.enabled: root.isSelectable

    // Only the left button drags
    mouseArea.drag.target: (root.mouseArea.pressedButtons & Qt.LeftButton) ? root : null
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

    background.anchors.fill: contentArea
    background.color: "transparent"
    background.border.width: 0
    background.topLeftRadius: root.isCardTop ? root.cardRadius : 0
    background.topRightRadius: root.isCardTop ? root.cardRadius : 0
    background.bottomLeftRadius: root.cardBottom || prv.dragged ? root.cardRadius : 0
    background.bottomRightRadius: root.cardBottom || prv.dragged ? root.cardRadius : 0

    // The height of the (collapsed) group this row is the header of: used for the card shown while dragging a group
    property real groupHeight: 0
    readonly property bool isDraggedGroup: prv.dragged && (root.type === LayoutPanelItemType.SHARED_PART || root.isEmptyGroup)

    StyledRectangularShadow {
        id: shadow

        anchors.fill: root.isDraggedGroup ? draggedGroupBackground : contentArea
        visible: false
        z: -2
    }

    // While a group is dragged: the whole (collapsed) group, with its background
    Rectangle {
        id: draggedGroupBackground

        width: root.width
        height: root.groupHeight
        z: -1

        visible: root.isDraggedGroup
        radius: 6
        color: ui.theme.backgroundPrimaryColor

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: ui.theme.buttonColor
            opacity: 0.5
        }

        // An empty group's "Drag instruments here" row
        Rectangle {
            x: 4
            y: root.contentTopMargin + root.contentHeight + 4
            width: parent.width - 8
            height: root.contentHeight

            visible: root.isEmptyGroup
            radius: 3
            color: ui.theme.textFieldColor

            StyledTextLabel {
                anchors.centerIn: parent
                text: qsTrc("layoutpanel", "Drag instruments here")
            }
        }
    }

    // Card background: expanded instruments (with their staves), and instruments in a shared-staves group
    Rectangle {
        id: cardBackground

        anchors.fill: contentArea
        z: -1

        // The expanded card's colour is dropped on the hovered row, so that the (translucent) hover fill
        // looks the same as on a collapsed row; in groups that falls back to the instruments' card colour
        readonly property bool showsExpandedCard: root.isInExpandedCard && !root.isHovered

        visible: showsExpandedCard || (root.isInGroup && root.type !== LayoutPanelItemType.SHARED_PART && !root.isEmptyGroup)
                 || (prv.dragged && !root.isDraggedGroup)
        color: showsExpandedCard ? ui.theme.textFieldColor : ui.theme.backgroundPrimaryColor

        topLeftRadius: root.background.topLeftRadius
        topRightRadius: root.background.topRightRadius
        bottomLeftRadius: root.background.bottomLeftRadius
        bottomRightRadius: root.background.bottomRightRadius
    }

    Loader {
        id: popupLoader

        readonly property StyledPopupView openedPopup: popupLoader.item as StyledPopupView
        readonly property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

        function openPopup(comp: Component, btn: FlatButton, anchor: Item, item) {
            popupLoader.sourceComponent = comp
            if (!openedPopup) {
                return
            }

            // Open to the right of the button, top-aligned and flush with it, without an arrow
            openedPopup.showArrow = false
            openedPopup.placementPolicies = PopupView.PreferRight
            openedPopup.parent = anchor

            // Top-aligned with the button, but moved up as far as needed to stay within the window
            const popup = openedPopup
            const windowMargin = 8
            popup.y = Qt.binding(function() {
                const anchorTop = anchor.mapToItem(null, 0, 0).y
                const visibleHeight = popup.height - popup.padding * 2
                const overflow = anchorTop + visibleHeight - (Window.height - windowMargin)
                return -Math.max(0, Math.min(overflow, anchorTop - windowMargin))
            })
            openedPopup.needActiveFirstItem = btn.navigation.highlight

            openedPopup.load(item)

            openedPopup.closed.connect(function() {
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
            onReplaceInstrumentRequested: {
                // The popup would close when the dialog to select the new
                // instrument is shown; when it closes, it is unloaded, i.e.
                // deleted, which means that it is deleted while a signal
                // handler inside it is being executed. This causes a crash.
                // To prevent that, let the popup close itself, and perform the
                // actual operation "later", i.e. not (directly or indirectly)
                // inside the signal handler in the popup.
                Qt.callLater((root.item as PartTreeItem).replaceInstrument)
            }

            onResetAllFormattingRequested: {
                // Same as above
                Qt.callLater((root.item as PartTreeItem).resetAllFormatting)
            }
        }
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {}
    }

    Component {
        id: systemObjectsLayerSettingsComp

        SystemObjectsLayerSettingsPopup {}
    }

    // Shown instead of the system markings settings while the score has no system markings to configure
    Component {
        id: systemObjectsLayerHelpComp

        StyledPopupView {
            id: helpPopup

            property bool needActiveFirstItem: false

            contentWidth: 240
            contentHeight: helpLabel.implicitHeight

            function load(obj) {
            }

            StyledTextLabel {
                id: helpLabel

                width: parent.width

                text: qsTrc("layoutpanel", "System markings let you choose which staves show elements like tempo markings, rehearsal marks, system text, voltas, and jumps. Add one or more of these elements in the score to configure their visibility here.")
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
            }
        }
    }

    // High contrast: instrument card stroke, one outline around an instrument and (when expanded) its staves.
    // Drawn per row, with the edges between rows of the same card clipped away; the theme's border width
    // is only non-zero in high contrast themes
    Item {
        anchors.fill: contentArea
        z: 1

        visible: ui.theme.borderWidth > 0 && (root.type === LayoutPanelItemType.PART || root.depth > 0)
        clip: true

        Rectangle {
            readonly property int overflow: 4
            readonly property bool closesBottom: root.cardBottom || prv.dragged

            width: parent.width
            y: root.isCardTop ? 0 : -overflow
            height: parent.height + (root.isCardTop ? 0 : overflow) + (closesBottom ? 0 : overflow)

            radius: root.cardRadius
            color: "transparent"
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor
        }
    }

    Item {
        id: contentArea

        x: 4
        y: root.contentTopMargin
        width: root.width - 8
        height: root.contentHeight

        HoverHandler {
            id: rowHover
        }

        // Clicks on disabled controls do nothing, rather than (re)selecting the row
        MouseArea {
            x: rowLayout.x + eyeCell.x
            y: rowLayout.y + eyeCell.y
            width: eyeCell.width
            height: eyeCell.height
            enabled: visibilityButton.visible && !visibilityButton.enabled
        }

        MouseArea {
            x: rowLayout.x + expandCell.x
            y: rowLayout.y + expandCell.y
            width: expandCell.width
            height: expandCell.height
            enabled: expandButton.visible && !expandButton.enabled
        }

        MouseArea {
            x: rowLayout.x + sharedPartToggle.x
            y: rowLayout.y + sharedPartToggle.y
            width: sharedPartToggle.width
            height: sharedPartToggle.height
            enabled: sharedPartToggle.visible && !sharedPartToggle.enabled
        }


        RowLayout {
            id: rowLayout

            anchors.fill: parent
            anchors.leftMargin: root.contentIndent
            // Room after the group toggle; without it, the cog lines up with the instruments' cogs
            anchors.rightMargin: sharedPartToggle.visible ? 5 : 0

            spacing: 4

            // Visibility (or the system markings icon)
            Item {
                id: eyeCell

                Layout.preferredWidth: root.contentHeight
                Layout.fillHeight: true

                FlatButton {
                    id: visibilityButton

                    anchors.fill: parent

                    // Empty groups have no visibility to show
                    visible: root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER && !root.isEmptyGroup
                             && (root.type !== LayoutPanelItemType.SHARED_PART || root.isGroupCombined)
                             && !(root.hideDisabledEyes && root.isGroupCombined
                                  && root.type !== LayoutPanelItemType.SHARED_PART && !enabled)

                    // Instruments on combined shared staves (and their staves) take their visibility from the group,
                    // so their eyes are disabled (shown at the theme's disabled opacity), mirroring the instrument
                    readonly property bool mirrorsInstrument: root.isGroupCombined && root.depth > 0
                    readonly property bool isShownVisible: mirrorsInstrument ? root.isParentVisible
                                                                             : Boolean(root.item) && root.item.isVisible

                    enabled: root.isEnabled && !mirrorsInstrument

                    objectName: "VisibleBtn"
                    navigation.panel: root.navigation.panel
                    navigation.row: root.navigation.row
                    navigation.column: 1
                    accessible.name: (titleLabel.text ? titleLabel.text + ", " : "")
                                     + (isShownVisible ? qsTrc("ui", "Visible") : qsTrc("ui", "Hidden"))

                    transparent: true
                    icon: isShownVisible ? IconCode.EYE_OPEN : IconCode.EYE_CLOSED

                    onClicked: {
                        const isVisible = root.item && root.item.isVisible
                        if (root.isSelected) {
                            root.changeVisibilityOfSelectedRowsRequested(!isVisible)
                        } else {
                            root.changeVisibilityRequested(root.modelIndex, !isVisible)
                        }
                    }
                }

                StyledIconLabel {
                    anchors.centerIn: parent

                    visible: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER
                    // System markings above (down arrow) / below the bottom staff (up arrow) - MusescoreIcon glyphs without an IconCode yet
                    iconCode: root.isLastRow ? 0xF4CA : 0xF4C9
                }
            }

            // Expand / collapse (an empty cell for staves, so that their names line up with the instrument's);
            // 2px after the visibility cell rather than the row's usual spacing
            Item {
                id: expandCell

                Layout.preferredWidth: root.contentHeight
                Layout.fillHeight: true
                Layout.leftMargin: -2

                visible: root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER

                FlatButton {
                    id: expandButton

                    readonly property bool isGroup: root.type === LayoutPanelItemType.SHARED_PART || root.isEmptyGroup
                    readonly property bool expanded: isGroup ? root.isGroupExpanded : root.isExpanded

                    anchors.fill: parent

                    visible: root.depth === 0 && root.isExpandable

                    // Groups are always open while their instruments aren't combined
                    // (and empty groups always are)
                    enabled: !isGroup || (root.isGroupCombined && !root.isEmptyGroup)

                    objectName: "ExpandBtn"
                    navigation.panel: root.navigation.panel
                    navigation.row: root.navigation.row
                    navigation.column: 2
                    navigation.accessible.name: expanded
                                                //: Collapse a tree item
                                                ? qsTrc("global", "Collapse")
                                                //: Expand a tree item
                                                : qsTrc("global", "Expand")

                    transparent: true
                    icon: expanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

                    onClicked: {
                        if (isGroup) {
                            root.groupExpandToggled()
                        } else if (root.isExpanded) {
                            root.treeView.collapse(root.modelIndex)
                        } else {
                            root.treeView.expand(root.modelIndex)
                        }
                    }
                }
            }

            StyledTextLabel {
                id: titleLabel

                Layout.fillWidth: true
                Layout.leftMargin: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER ? -2 : 0

                text: root.item ? root.item.title : ""
                horizontalAlignment: Text.AlignLeft

                readonly property bool isItemVisible: Boolean(root.item) && root.item.isVisible
                readonly property bool isBold: {
                    switch (root.type) {
                    case LayoutPanelItemType.PART:
                        // Instruments on combined shared staves are shown as regular text under their (bold) group
                        return isItemVisible && !root.isGroupCombined
                    case LayoutPanelItemType.SHARED_PART:
                        return isItemVisible && root.isGroupCombined
                    case LayoutPanelItemType.EMPTY_SHARED_GROUP:
                        return true
                    default:
                        return false
                    }
                }
                readonly property bool isDimmed: !isItemVisible && root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER
                                                 || (root.type === LayoutPanelItemType.SHARED_PART && !root.isGroupCombined)

                font: isBold ? ui.theme.bodyBoldFont : ui.theme.bodyFont
                opacity: isDimmed ? 0.7 : 1
            }

            FlatButton {
                id: settingsButton

                Layout.preferredWidth: root.contentHeight
                Layout.preferredHeight: root.contentHeight

                // System markings rows keep their cog enabled when there's nothing to configure yet: it then explains them
                readonly property bool showsSystemObjectsHelp: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER && !root.settingsEnabled

                visible: root.settingsAvailable
                enabled: root.visible && (root.settingsEnabled || showsSystemObjectsHelp)

                // Shown on hover / press / selection, and while the row or any of its controls has keyboard focus;
                // kept in the layout (transparent) so that it stays keyboard-accessible
                readonly property bool rowHasKeyboardFocus: root.navigation.highlight || navigation.highlight
                                                            || visibilityButton.navigation.highlight
                                                            || expandButton.navigation.highlight
                                                            || sharedPartToggle.navigation.highlight

                opacity: !root.onlyShowSettingsOnHover || root.isHovered || root.mouseArea.pressed || root.isSelected
                         || popupLoader.isPopupOpened || groupMenu.isMenuOpened || rowHasKeyboardFocus ? 1 : 0

                objectName: "SettingsBtn"
                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 3
                navigation.accessible.name: qsTrc("layoutpanel", "Settings")

                // Accent state while its popup (or a group's menu) is open
                transparent: !popupLoader.isPopupOpened && !groupMenu.isMenuOpened
                accentButton: popupLoader.isPopupOpened || groupMenu.isMenuOpened
                icon: IconCode.SETTINGS_COG

                // The popup window has a padding around its visible content (for the shadow); end this anchor
                // that much before the button's right edge, so that the visible popup sits flush against the button
                Item {
                    id: popupAnchor

                    width: parent.width - (popupLoader.openedPopup ? popupLoader.openedPopup.padding : 0)
                    height: parent.height
                }

                // Groups' cog opens a menu (for now) rather than a settings popup
                StyledMenuLoader {
                    id: groupMenu

                    menuAnchorItem: root.Window.window ? root.Window.window.contentItem : null
                    placementPolicies: PopupView.PreferRight

                    onHandleMenuItem: function(itemId) {
                        if (itemId === "ungroup-instruments") {
                            root.ungroupRequested()
                        } else if (itemId === "delete-group") {
                            root.deleteEmptyGroupRequested()
                        }
                    }
                }

                onClicked: {
                    if (popupLoader.isPopupOpened) {
                        popupLoader.closeOpenedPopup()
                        return
                    }

                    if (root.type === LayoutPanelItemType.SHARED_PART) {
                        // A shared part always has at least one instrument
                        groupMenu.toggleOpened([
                            { id: "ungroup-instruments", title: qsTrc("layoutpanel", "Ungroup instruments") }
                        ])
                        return
                    }

                    if (root.isEmptyGroup) {
                        groupMenu.toggleOpened([
                            { id: "delete-group", title: qsTrc("layoutpanel", "Delete group"), icon: IconCode.DELETE_TANK }
                        ])
                        return
                    }

                    let comp = null
                    let item = {}

                    if (root.type === LayoutPanelItemType.PART) {
                        comp = instrumentSettingsComp

                        item["partId"] = root.item.id
                        item["instrumentId"] = (root.item as PartTreeItem).instrumentId()
                    } else if (root.type === LayoutPanelItemType.STAFF) {
                        comp = staffSettingsComp

                        item["id"] = root.item.id
                    } else if (showsSystemObjectsHelp) {
                        comp = systemObjectsLayerHelpComp
                    } else if (root.type == LayoutPanelItemType.SYSTEM_OBJECTS_LAYER) {
                        comp = systemObjectsLayerSettingsComp

                        item["staffId"] = (root.item as SystemObjectsLayerTreeItem).staffId()
                    }

                    popupLoader.openPopup(comp, this, popupAnchor, item)
                }
            }

            ToggleButton {
                id: sharedPartToggle

                // The per-group switch only exists while instruments are being combined; an empty group's switch
                // is always shown, disabled, reflecting whether instruments are being combined
                visible: (root.type === LayoutPanelItemType.SHARED_PART && root.isStaveSharingEnabled) || root.isEmptyGroup
                enabled: !root.isEmptyGroup

                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 4
                navigation.accessible.name: toolTipTitle
                toolTipTitle: qsTrc("layoutpanel", "Combine onto shared staves")

                checked: root.isEmptyGroup ? root.isStaveSharingEnabled : Boolean(root.item) && root.item.isEnabled
                onToggled: function() {
                    if (root.isSelected) {
                        root.changeEnabledOfSelectedRowsRequested(!checked)
                    } else {
                        root.changeEnabledRequested(root.modelIndex, !checked)
                    }
                }
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
            when: root.usesRowHighlight && !root.isLocked && root.isHovered && !root.mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.usesRowHighlight && !root.isLocked && root.mouseArea.pressed && !root.isSelected && !prv.dragged

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.usesRowHighlight && root.isSelected && !root.isHovered && !root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: root.usesRowHighlight && root.isSelected && root.isHovered && !root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
            }
        },

        State {
            name: "SELECTED_PRESSED"
            when: root.usesRowHighlight && root.isSelected && root.mouseArea.pressed

            PropertyChanges {
                target: root.background
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
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
                opacity: 0.9
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
