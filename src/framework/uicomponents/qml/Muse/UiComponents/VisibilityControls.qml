/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

RowLayout {
    id: root

    spacing: 2

    required property NavigationPanel navigationPanel
    required property int navigationRow

    property alias title: titleLabel.text
    property bool isRootControl: true

    property alias useVisibilityButton: visibilityBox.visible
    property alias isVisible: visibilityBox.isVisible
    property alias canChangeVisibility: visibilityBox.enabled

    property bool showDashIcon: false

    property bool isExpandable: false
    property bool isExpanded: false
    property bool makeRoomForExpandButton: root.expandableDepth !== 0
    property int expandableDepth: 0

    signal visibilityButtonClicked(var isVisibile);
    signal expandButtonClicked(var expand)
    signal settingsButtonClicked(var button)

    VisibilityBox {
        id: visibilityBox

        Layout.alignment: Qt.AlignLeft
        Layout.preferredWidth: width

        objectName: "VisibleBtn"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRow
        navigation.column: 1

        onVisibleToggled: {
            root.visibilityButtonClicked(visibilityBox.isVisible)
        }
    }

    StyledIconLabel {
        id: dashIcon

        Layout.preferredWidth: visibilityBox.width
        Layout.alignment: Qt.AlignCenter

        visible: root.showDashIcon

        iconCode: IconCode.MINUS
        opacity: root.isVisible ? 1 : 0.75
    }

    Item {
        Layout.fillWidth: true
        Layout.leftMargin: 12 * root.expandableDepth
        height: childrenRect.height

        FlatButton {
            id: expandButton

            anchors.left: parent.left

            visible: root.isExpandable

            width: expandButton.visible || root.makeRoomForExpandButton ? expandButton.implicitWidth : 0

            objectName: "ExpandBtn"
            enabled: expandButton.visible
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 2
            navigation.accessible.name: root.isExpanded
                                        //: Collapse a tree item
                                        ? qsTrc("global", "Collapse")
                                        : qsTrc("global", "Expand")

            transparent: true
            icon: root.isExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

            onClicked: {
                root.expandButtonClicked(!root.isExpanded)
            }
        }

        // FIX 3: Placed Icon INSIDE the Item, between Button and Text.
        StyledIconLabel {
            id: linkIcon
            
            visible: root.isLinked
            
            // Anchor to the expand button
            anchors.left: expandButton.right
            anchors.verticalCenter: expandButton.verticalCenter
            
            iconCode: IconCode.LINK 
            
            // Standard size for these icons
            width: 24
            height: 24

            color: ui.theme.iconColor
            
            // Accessibility Requirement
            accessible.name: qsTrc("layoutpanel", "Linked staff")
        }

        StyledTextLabel {
            id: titleLabel

            // FIX 4: Smart anchoring. 
            // If icon is visible, anchor to icon. If not, anchor to button.
            anchors.left: linkIcon.visible ? linkIcon.right : expandButton.right
            anchors.leftMargin: linkIcon.visible ? 0 : 4
            
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: expandButton.verticalCenter

            horizontalAlignment: Text.AlignLeft
            opacity: root.isVisible ? 1 : 0.75

            font: root.isRootControl && root.isVisible ? ui.theme.bodyBoldFont : ui.theme.bodyFont
        }
    }
}
