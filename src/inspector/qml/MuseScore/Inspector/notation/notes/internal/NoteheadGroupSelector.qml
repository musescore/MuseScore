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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Muse.UiComponents
import Muse.Ui
import MuseScore.Inspector

import "../../../common"

InspectorPropertyView {
    id: root

    titleText: qsTrc("inspector", "Notehead type")

    navigationRowEnd: navigationRowStart + gridView.count + 1 /*menu button*/

    FocusableItem {
        width: parent.width
        height: gridView.implicitHeight + 2 * gridView.anchors.margins

        Rectangle {
            anchors.fill: parent

            color: ui.theme.textFieldColor
            radius: 3
        }

        StyledGridView {
            id: gridView
            anchors.fill: parent
            anchors.margins: 8

            implicitHeight: Math.min(contentHeight, 3 * cellHeight)

            readonly property int cellRadius: 2

            cellHeight: 40
            cellWidth: 40

            model: NoteheadGroupsModel {
                id: noteheadGroupsModel
            }

            delegate: ListItemBlank {
                id: delegateItem
                implicitHeight: gridView.cellHeight
                implicitWidth: gridView.cellWidth

                required property int headGroup
                required property string headHint
                required property int iconCode
                required property int index

                hint: headHint
                background.radius: gridView.cellRadius

                navigation.name: hint
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart + 1 + index
                navigation.accessible.name: hint
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        gridView.positionViewAtIndex(index, ListView.Contain)
                    }
                }

                StyledIconLabel {
                    anchors.centerIn: parent

                    // TODO: Replace with ui.theme.musicalIconsFont.family when Leland supports these icons
                    font.family: "Bravura"
                    font.pixelSize: 30

                    iconCode: delegateItem.iconCode
                }

                onClicked: {
                    if (root.propertyItem) {
                        root.propertyItem.value = delegateItem.headGroup
                    }
                }
            }

            highlight: Rectangle {
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
                radius: gridView.cellRadius
            }

            currentIndex: root.propertyItem && !root.propertyItem.isUndefined
                          ? root.propertyItem.value
                          : -1

            ScrollBar.vertical: StyledScrollBar {
                policy: ScrollBar.AlwaysOn
            }
        }
    }
}
