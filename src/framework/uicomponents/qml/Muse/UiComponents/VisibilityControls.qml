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

import QtQuick 2.15
import QtQuick.Layouts

import Muse.Ui 1.0
import Muse.UiComponents 1.0

RowLayout {
    id: root

    spacing: 2

    required property NavigationPanel navigationPanel
    required property int navigationRow

    property alias title: titleLabel.text
    property bool isRootControl: true

    property alias useVisibilityButton: visibilityBox.visible
    property alias isVisible: visibilityBox.isVisible

    property bool showDashIcon: false

    property bool isExpandable: false
    property bool isExpanded: false
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

            width: expandButton.visible || root.expandableDepth !== 0 ? expandButton.implicitWidth : 0

            objectName: "ExpandBtn"
            enabled: expandButton.visible
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRow
            navigation.column: 2
            navigation.accessible.name: root.isExpanded
                                        //: Collapse a tree item
                                        ? qsTrc("global", "Collapse")
                                        //: Expand a tree item
                                        : qsTrc("global", "Expand")

            transparent: true
            icon: root.isExpanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT

            onClicked: {
                root.expandButtonClicked(!root.isExpanded)
            }
        }

        StyledTextLabel {
            id: titleLabel

            anchors.left: expandButton.right
            anchors.leftMargin: 4
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: expandButton.verticalCenter

            horizontalAlignment: Text.AlignLeft
            opacity: root.isVisible ? 1 : 0.75

            font: root.isRootControl && root.isVisible ? ui.theme.bodyBoldFont : ui.theme.bodyFont
        }
    }
}
