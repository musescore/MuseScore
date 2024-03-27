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

import MuseScore.Ui
import MuseScore.UiComponents

Item {
    id: root

    property alias model: treeView.model

    property NavigationPanel navigation: NavigationPanel {
        name: "PreferencesMenuPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlIndex", prv.currentItemNavigationIndex)
            }
        }
    }

    QtObject {
        id: prv

        property var currentItemNavigationIndex: []
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    TreeView {
        id: treeView

        anchors.fill: parent
        anchors.topMargin: 12

        boundsBehavior: Flickable.StopAtBounds

        delegate: PageTabButton {
            required property bool expanded
            required property int depth
            required property int row
            required property int column

            readonly property var modelIndex: treeView.index(row, column)

            readonly property int navigationRow: modelIndex.row
            readonly property int navigationColumn: depth

            implicitWidth: treeView.width
            implicitHeight: 36

            orientation: Qt.Horizontal

            spacing: 16
            leftPadding: spacing * (depth + 1)

            normalStateFont: ui.theme.bodyFont
            selectedStateFont: ui.theme.bodyBoldFont

            title: Boolean(model) ? model.itemRole.title : ""
            checked: Boolean(model) && Boolean(model.itemRole) ? model.itemRole.id === treeView.model.currentPageId : false
            enabled: visible

            navigation.name: "PreferencesMenuItem"
            navigation.panel: root.navigation
            navigation.row: navigationRow
            navigation.column: navigationColumn
            navigation.accessible.name: title
            navigation.accessible.role: MUAccessible.ListItem
            navigation.onActiveChanged: {
                if (navigation.active) {
                    treeView.model.selectRow(modelIndex)
                }
            }

            readonly property bool shouldBeExpanded: model?.itemRole?.expanded ?? false

            function updateExpandedState() {
                if (shouldBeExpanded) {
                    treeView.expand(row)
                } else {
                    treeView.collapse(row)
                }
            }

            Component.onCompleted: {
                Qt.callLater(updateExpandedState)
            }

            onShouldBeExpandedChanged: {
                Qt.callLater(updateExpandedState)
            }

            iconComponent: StyledIconLabel {
                width: 24
                height: width
                iconCode: Boolean(model) ? model.itemRole.icon : IconCode.NONE
            }

            onCheckedChanged: {
                if (checked) {
                    prv.currentItemNavigationIndex = [navigationRow, navigationColumn]
                }
            }

            onClicked: {
                treeView.model.selectRow(modelIndex)
            }
        }
    }
}
