/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

//! A row: an instrument, or a shared-staves group's header or instrument
ListItemBlank {
    id: root

    required isSelected
    required property string name
    required property string description
    required property bool isSoloist
    required property int index
    required property int sharedGroupRole
    required property string sharedGroupId

    required property NavigationPanel navigationPanel

    // Shared by all rows of the group, owned by the view
    property bool isGroupExpanded: false
    property bool isGroupSelected: false
    property bool isGroupHovered: false
    property bool isGroupPressed: false

    readonly property bool isInGroup: root.sharedGroupRole !== InstrumentsOnScoreListModel.NotInGroup
    readonly property bool isGroupHeader: root.sharedGroupRole === InstrumentsOnScoreListModel.GroupHeader
    readonly property bool isGroupMember: root.isInGroup && !root.isGroupHeader

    signal groupExpandToggled()
    signal groupHoverChanged(bool hovered)
    signal groupPressChanged(bool pressed)

    QtObject {
        id: prv

        readonly property int rowHeight: 30
        readonly property int groupPadding: 4
        readonly property int groupGap: 8
        readonly property int groupRadius: 6
        readonly property int cardRadius: 3

        readonly property bool isHidden: root.isGroupMember && !root.isGroupExpanded
        readonly property bool isGroupEnd: root.sharedGroupRole === InstrumentsOnScoreListModel.GroupLastMember
                                           || (root.isGroupHeader && !root.isGroupExpanded)

        readonly property int contentTop: root.isGroupHeader ? prv.groupPadding : 0
        readonly property int contentBottom: root.isInGroup ? prv.groupPadding : 0
        readonly property int gap: prv.isGroupEnd ? prv.groupGap : 0
    }

    height: prv.isHidden ? 0 : prv.contentTop + prv.rowHeight + prv.contentBottom + prv.gap
    visible: !prv.isHidden

    // In a group, the row's own state is shown on its card; a header's is shown on the whole group
    background.anchors.fill: contentArea
    background.visible: !root.isGroupHeader
    radius: root.isInGroup ? prv.cardRadius : 0
    mouseArea.anchors.fill: contentArea

    navigation.name: root.name
    navigation.enabled: !prv.isHidden
    navigation.panel: root.navigationPanel
    navigation.row: 1 + root.index
    navigation.column: 0
    navigation.accessible.name: itemTitleLabel.text
    navigation.accessible.description: root.description
    navigation.accessible.row: root.index

    onHovered: function(isHovered) {
        if (root.isInGroup) {
            root.groupHoverChanged(isHovered)
        }
    }

    onPressed: function(isPressed) {
        if (root.isGroupHeader) {
            root.groupPressChanged(isPressed)
        }
    }

    onDoubleClicked: {
        // a header expands / collapses (removing it would ungroup its instruments)
        if (root.isGroupHeader) {
            root.groupExpandToggled()
            return
        }

        root.removeSelectionRequested()
    }

    // the group's background: rounded at the group's top and bottom
    Rectangle {
        anchors.top: root.top
        anchors.left: root.left
        anchors.right: root.right
        height: root.height - prv.gap
        z: -2

        visible: root.isInGroup

        topLeftRadius: root.isGroupHeader ? prv.groupRadius : 0
        topRightRadius: root.isGroupHeader ? prv.groupRadius : 0
        bottomLeftRadius: prv.isGroupEnd ? prv.groupRadius : 0
        bottomRightRadius: prv.isGroupEnd ? prv.groupRadius : 0

        color: root.isGroupSelected ? ui.theme.accentColor : ui.theme.buttonColor
        opacity: {
            if (root.isGroupPressed) {
                return 0.7
            }

            if (root.isGroupHovered) {
                return root.isGroupSelected ? 0.4 : 0.3
            }

            return 0.5
        }
    }

    Rectangle {
        anchors.fill: contentArea
        z: -1

        visible: root.isGroupMember
        radius: prv.cardRadius
        color: ui.theme.backgroundPrimaryColor
    }

    Item {
        id: contentArea

        x: root.isInGroup ? prv.groupPadding : 0
        y: prv.contentTop
        width: root.width - 2 * contentArea.x
        height: prv.rowHeight

        FlatButton {
            id: expandButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: prv.rowHeight
            height: prv.rowHeight

            visible: root.isGroupHeader
            transparent: true
            icon: root.isGroupExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

            navigation.name: root.name + "Expand"
            navigation.enabled: expandButton.visible
            navigation.panel: root.navigationPanel
            navigation.row: 1 + root.index
            navigation.column: 1
            navigation.accessible.name: root.isGroupExpanded
                                        //: Collapse a tree item
                                        ? qsTrc("global", "Collapse")
                                        //: Expand a tree item
                                        : qsTrc("global", "Expand")

            onClicked: {
                root.groupExpandToggled()
            }
        }

        StyledTextLabel {
            id: itemTitleLabel

            anchors.left: parent.left
            // A group's name follows its expand button; its instruments line up with the name
            anchors.leftMargin: root.isInGroup ? prv.rowHeight + 4 : 12
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter

            horizontalAlignment: Text.AlignLeft
            text: root.isSoloist ? qsTrc("instruments", "Soloist:") + " " + root.name : root.name
            font: root.isGroupMember ? ui.theme.bodyFont : ui.theme.bodyBoldFont
        }

        // headers hide their background, and with it the focus border
        NavigationFocusBorder {
            navigationCtrl: root.navigation
            drawOutsideParent: false
            visible: root.isGroupHeader && root.navigation.highlight
            radius: prv.cardRadius
        }
    }
}
