/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0
import MuseScore.NotationScene 1.0

DropArea {
    id: root

    property var padModel: null

    property bool panelEnabled: false

    property int panelMode: -1
    property bool useNotationPreview: false
    property int notationPreviewNumStaffLines: 0
    property bool showEditOutline: false

    property alias totalBorderWidth: padLoader.anchors.margins
    property alias showOriginBackground: originBackground.visible
    property alias swappableArea: swappableArea

    property int navigationRow: -1
    property int navigationColumn: -1
    property alias padNavigation: padNavCtrl
    property alias footerNavigation: footerNavCtrl

    readonly property bool hasActiveControl: padNavCtrl.active || footerNavCtrl.active

    property bool panelHasActiveKeyboardSwap: false

    property var dragParent: null
    signal startPadSwapRequested(var isKeyboardSwap)
    signal endPadSwapRequested()
    signal cancelPadSwapRequested()

    onDropped: function(dropEvent)  {
        root.endPadSwapRequested()
        dropEvent.accepted = true
    }

    QtObject {
        id: prv
        readonly property color enabledBackgroundColor: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityNormal)
        readonly property color disabledBackgroundColor: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.itemOpacityDisabled)
        readonly property real footerHeight: 24

        readonly property string accessibleDetailsString: {
            if (!Boolean(root.padModel)) {
                return ""
            }

            //: %1 will be the MIDI note for a drum (displayed in the percussion panel)
            let line1 = qsTrc("notation/percussion", "MIDI %1").arg(root.padModel.midiNote)

            let shortcut = root.padModel.keyboardShortcut
            if (shortcut === "") {
                return line1
            }


            //: %1 will be the shortcut for a drum (displayed in the percussion panel)
            let line2 = qsTrc("notation/percussion", "Shortcut %1").arg(shortcut)

            return line2 + ", " + line1
        }

        readonly property string accessibleRowColumnString: {
            //: %1 will be the row number of a percussion panel pad
            let line1 = qsTrc("notation/percussion", "Row %1").arg(root.navigationRow + 1)

            //: %1 will be the column number of a percussion panel pad
            let line2 = qsTrc("notation/percussion", "Column %1").arg(root.navigationColumn + 1)

            return line1 + " " + line2
        }

        readonly property string fullAccessibleString: prv.accessibleDetailsString + ", " + prv.accessibleRowColumnString
    }

    NavigationControl {
        id: padNavCtrl

        row: root.navigationRow
        column: root.navigationColumn

        name: root.objectName !== "" ? root.objectName : "PercussionPanelPad"

        // Only navigate to empty slots when we're in edit mode
        enabled: Boolean(root.padModel) || root.panelMode === PanelMode.EDIT_LAYOUT

        accessible.role: MUAccessible.SilentRole
        accessible.name: Boolean(root.padModel) ? root.padModel.padName : qsTrc("notation/percussion", "Empty pad")

        accessible.description: Boolean(root.padModel) ? prv.fullAccessibleString : prv.accessibleRowColumnString

        accessible.visualItem: padFocusBorder
        accessible.enabled: padNavCtrl.enabled

        onTriggered: {
            if (Boolean(root.padModel) && root.panelMode !== PanelMode.EDIT_LAYOUT) {
                root.padModel.triggerPad()
                return
            }
            root.panelHasActiveKeyboardSwap ? root.endPadSwapRequested() : root.startPadSwapRequested(true)
        }
    }

    NavigationControl {
        id: footerNavCtrl

        row: root.navigationRow
        column: root.navigationColumn

        name: root.objectName !== "" ? root.objectName : "PercussionPanelPadFooter"

        enabled: Boolean(root.padModel)

        accessible.role: MUAccessible.SilentRole
        accessible.name: Boolean(root.padModel) ? root.padModel.padName + " " + qsTrc("notation/percussion", "options") : ""

        accessible.visualItem: footerFocusBorder
        accessible.enabled: footerNavCtrl.enabled
    }

    Rectangle {
        id: swappableArea

        // Protrudes slightly from behind the components in the loader to produce the edit mode "border with gap" effect
        width: root.width
        height: root.height

        radius: root.width / 6

        color: ui.theme.backgroundPrimaryColor

        border.color: root.showEditOutline ? ui.theme.accentColor : "transparent"
        border.width: 2

        DragHandler {
            id: dragHandler

            target: swappableArea
            enabled: Boolean(root.padModel) && root.panelMode === PanelMode.EDIT_LAYOUT && !root.panelHasActiveKeyboardSwap

            dragThreshold: 0 // prevents the flickable from stealing drag events

            onActiveChanged: {
                if (dragHandler.active) {
                    root.startPadSwapRequested(false)
                    return
                }
                if (!swappableArea.Drag.drop()) {
                    root.cancelPadSwapRequested()
                }
            }
        }

        Drag.active: dragHandler.active
        Drag.hotSpot.x: root.width / 2
        Drag.hotSpot.y: root.height / 2

        Loader {
            // Loads either an empty slot or the pad content
            id: padLoader

            anchors.fill: parent
            // Defined as 1 in the spec, but causes some aliasing in practice...
            anchors.margins: 2 + swappableArea.border.width

            // Can't simply use clip as this won't take into account radius...
            layer.enabled: ui.isEffectsAllowed
            layer.effect: EffectOpacityMask {
                maskSource: Rectangle {
                    width: padLoader.width
                    height: padLoader.height
                    radius: swappableArea.radius - padLoader.anchors.margins
                }
            }

            sourceComponent: Boolean(root.padModel) ? padContentComponent : emptySlotComponent

            Component {
                id: padContentComponent

                PercussionPanelPadContent {
                    id: padContent

                    padModel: root.padModel
                    panelMode: root.panelMode
                    useNotationPreview: root.useNotationPreview
                    notationPreviewNumStaffLines: root.notationPreviewNumStaffLines

                    footerHeight: prv.footerHeight

                    padSwapActive: dragHandler.active

                    Connections {
                        target: footerNavCtrl
                        function onTriggered() {
                            padContent.openContextMenu(null)
                        }
                    }
                }
            }

            Component {
                id: emptySlotComponent

                Rectangle {
                    id: emptySlotBackground
                    color: root.panelEnabled ? prv.enabledBackgroundColor : prv.disabledBackgroundColor
                }
            }
        }

        NavigationFocusBorder {
            id: footerFocusBorder

            anchors {
                fill: null

                left: padLoader.left
                right: padLoader.right
                bottom: padLoader.bottom
            }

            height: prv.footerHeight + root.totalBorderWidth
            radius: 0

            navigationCtrl: footerNavCtrl
        }

        states: [
            State {
                name: "DRAGGED"
                when: dragHandler.active
                ParentChange {
                    target: swappableArea
                    parent: root.dragParent
                }
                AnchorChanges {
                    target: swappableArea
                    anchors.horizontalCenter: undefined
                    anchors.verticalCenter: undefined
                }
            },
            //! NOTE: Workaround for a bug in Qt 6.2.4 - see PR #24106 comment
            // https://bugreports.qt.io/browse/QTBUG-99436
            State {
                name: "DROPPED"
                when: !dragHandler.active
                ParentChange {
                    target: swappableArea
                    parent: root
                }
            }
        ]
    }

    //! NOTE: Ideally this would be swappableArea's child, but we don't want it to move when "previewing" a drop
    NavigationFocusBorder {
        id: padFocusBorder

        property real extraSize: root.showEditOutline ? padFocusBorder.border.width * 2 : 0
        property real extraRadius: root.showEditOutline ? root.totalBorderWidth + padFocusBorder.anchors.margins : 0

        anchors {
            fill: null
            centerIn: parent
        }

        width: swappableArea.width + padFocusBorder.extraSize
        height: swappableArea.height + padFocusBorder.extraSize

        radius: swappableArea.radius + padFocusBorder.extraRadius

        padding: root.showEditOutline ? 0 : root.totalBorderWidth * -1
        navigationCtrl: padNavCtrl
    }

    Rectangle {
        id: originBackground

        anchors.fill: parent

        radius: swappableArea.radius

        border.color: swappableArea.border.color
        border.width: swappableArea.border.width

        color: swappableArea.color

        Rectangle {
            id: originBackgroundFill

            anchors.fill: parent
            anchors.margins: padLoader.anchors.margins
            radius: swappableArea.radius - originBackgroundFill.anchors.margins

            color: root.containsDrag ? ui.theme.buttonColor : prv.enabledBackgroundColor
        }
    }
}
