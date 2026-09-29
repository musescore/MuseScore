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

    property alias isPopupOpened: popupLoader.isPopupOpened

    readonly property bool isCardTop: root.depth === 0
    readonly property bool isInExpandedCard: root.depth > 0 || (root.isExpanded && root.type === LayoutPanelItemType.PART)
    readonly property bool isHovered: rowHover.hovered && !prv.dragged

    // Group headers show their hover / pressed / selected state on the whole group's background (see LayoutPanel.qml)
    readonly property bool usesRowHighlight: root.type !== LayoutPanelItemType.SHARED_PART
    readonly property int cardRadius: 3

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal removeSelectionRequested()
    signal groupExpandToggled()

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
    navigation.accessible.name: titleLabel.text

    onNavigationTriggered: { root.clicked(null) }

    mouseArea.anchors.fill: contentArea
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

    background.anchors.fill: contentArea
    background.color: "transparent"
    background.border.width: 0
    background.topLeftRadius: root.isCardTop ? root.cardRadius : 0
    background.topRightRadius: root.isCardTop ? root.cardRadius : 0
    background.bottomLeftRadius: root.cardBottom ? root.cardRadius : 0
    background.bottomRightRadius: root.cardBottom ? root.cardRadius : 0

    StyledRectangularShadow {
        id: shadow

        anchors.fill: contentArea
        visible: false
        z: -2
    }

    // Card background: expanded instruments (with their staves), and instruments in a shared-staves group
    Rectangle {
        id: cardBackground

        anchors.fill: contentArea
        z: -1

        visible: root.isInExpandedCard || (root.isInGroup && root.type === LayoutPanelItemType.PART) || prv.dragged
        color: root.isInExpandedCard ? ui.theme.textFieldColor : ui.theme.backgroundPrimaryColor

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
        id: sharedPartSettingsComp

        SharedPartSettingsPopup {}
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {}
    }

    Component {
        id: systemObjectsLayerSettingsComp

        SystemObjectsLayerSettingsPopup {}
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

        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: root.type === LayoutPanelItemType.SHARED_PART ? 5 : 0

            spacing: 4

            // Visibility (or the system markings icon)
            Item {
                Layout.preferredWidth: root.contentHeight
                Layout.fillHeight: true

                FlatButton {
                    id: visibilityButton

                    anchors.fill: parent

                    visible: root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER
                             && (root.type !== LayoutPanelItemType.SHARED_PART || root.isGroupCombined)

                    // Instruments on combined shared staves take their visibility from the group
                    enabled: root.isEnabled

                    objectName: "VisibleBtn"
                    navigation.panel: root.navigation.panel
                    navigation.row: root.navigation.row
                    navigation.column: 1
                    accessible.name: (titleLabel.text ? titleLabel.text + ", " : "")
                                     + (root.item && root.item.isVisible ? qsTrc("ui", "Visible") : qsTrc("ui", "Hidden"))

                    transparent: true
                    icon: root.item && root.item.isVisible ? IconCode.EYE_OPEN : IconCode.EYE_CLOSED

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

            // Expand / collapse (an empty cell for staves, so that their names line up with the instrument's)
            Item {
                Layout.preferredWidth: root.contentHeight
                Layout.fillHeight: true

                visible: root.type !== LayoutPanelItemType.SYSTEM_OBJECTS_LAYER

                FlatButton {
                    id: expandButton

                    readonly property bool isGroup: root.type === LayoutPanelItemType.SHARED_PART
                    readonly property bool expanded: isGroup ? root.isGroupExpanded : root.isExpanded

                    anchors.fill: parent

                    visible: root.depth === 0 && root.isExpandable

                    // Groups are always open while their instruments aren't combined
                    enabled: !isGroup || root.isStaveSharingEnabled

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

                visible: root.settingsAvailable
                enabled: root.visible && root.settingsEnabled

                // Only shown on hover / press / selection; kept in the layout (transparent) so that it stays keyboard-accessible
                opacity: root.isHovered || root.mouseArea.pressed || root.isSelected || popupLoader.isPopupOpened || navigation.highlight ? 1 : 0

                objectName: "SettingsBtn"
                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 3
                navigation.accessible.name: qsTrc("layoutpanel", "Settings")

                // Accent state while its popup is open
                transparent: !popupLoader.isPopupOpened
                accentButton: popupLoader.isPopupOpened
                icon: IconCode.SETTINGS_COG

                // The popup window has a padding around its visible content (for the shadow); end this anchor
                // that much before the button's right edge, so that the visible popup sits flush against the button
                Item {
                    id: popupAnchor

                    width: parent.width - (popupLoader.openedPopup ? popupLoader.openedPopup.padding : 0)
                    height: parent.height
                }

                onClicked: {
                    if (popupLoader.isPopupOpened) {
                        popupLoader.closeOpenedPopup()
                        return
                    }

                    let comp = null
                    let item = {}

                    if (root.type === LayoutPanelItemType.SHARED_PART) {
                        comp = sharedPartSettingsComp
                    } else if (root.type === LayoutPanelItemType.PART) {
                        comp = instrumentSettingsComp

                        item["partId"] = root.item.id
                        item["instrumentId"] = (root.item as PartTreeItem).instrumentId()
                    } else if (root.type === LayoutPanelItemType.STAFF) {
                        comp = staffSettingsComp

                        item["id"] = root.item.id
                    } else if (root.type == LayoutPanelItemType.SYSTEM_OBJECTS_LAYER) {
                        comp = systemObjectsLayerSettingsComp

                        item["staffId"] = (root.item as SystemObjectsLayerTreeItem).staffId()
                    }

                    popupLoader.openPopup(comp, this, popupAnchor, item)
                }
            }

            ToggleButton {
                id: sharedPartToggle

                // The per-group switch only exists while instruments are being combined
                visible: root.type === LayoutPanelItemType.SHARED_PART && root.isStaveSharingEnabled

                navigation.panel: root.navigation.panel
                navigation.row: root.navigation.row
                navigation.column: 4
                navigation.accessible.name: qsTrc("layoutpanel", "Combine onto shared staves")

                checked: root.item && root.item.isEnabled
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
            when: root.usesRowHighlight && root.isHovered && !root.mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: root.background
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.usesRowHighlight && root.mouseArea.pressed && !root.isSelected && !prv.dragged

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
