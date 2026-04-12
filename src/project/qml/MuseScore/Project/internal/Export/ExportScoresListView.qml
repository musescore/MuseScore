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
import MuseScore.Project

Rectangle {
    id: root

    property alias scoresModel: listView.model
    property alias navigation: navPanel

    color: ui.theme.textFieldColor
    border.width: 1
    border.color: ui.theme.strokeColor

    function focusOnFirst() {
        listView.itemAtIndex(0).requestActiveFocus()
    }

    NavigationPanel {
        id: navPanel
        name: "ExportScoresListView"
        enabled: root.visible && root.enabled
        direction: NavigationPanel.Vertical
    }

    StyledListView {
        id: listView

        anchors.fill: parent
        anchors.margins: root.border.width

        delegate: ListItemBlank {
            id: delegateItem

            required property string title
            required property bool isMain
            required property bool isSelected
            required property int index

            mouseArea.hoverEnabled: false
            hoverHitColor: "transparent"

            function requestActiveFocus() {
                checkBox.navigation.requestActive()
            }

            CheckBox {
                id: checkBox
                anchors.margins: 4
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right

                text: delegateItem.isMain ? qsTrc("project/export", "Main score") : delegateItem.title
                font: delegateItem.isMain ? ui.theme.bodyBoldFont : ui.theme.bodyFont

                navigation.name: "ExportScoreCheckBox " + text
                navigation.panel: navPanel
                navigation.row: delegateItem.index
                navigation.column: 0

                checked: delegateItem.isSelected

                onClicked: {
                    root.scoresModel.setSelected(delegateItem.index, !checked)
                }

                navigation.onNavigationEvent: function(event) {
                    let targetIndex = delegateItem.index

                    switch (event.type) {
                    case NavigationEvent.Up:
                        if (delegateItem.index === 0) {
                            event.accepted = true
                            return
                        }
                        targetIndex = delegateItem.index - 1
                        break

                    case NavigationEvent.Down:
                        if (delegateItem.index === listView.count - 1) {
                            event.accepted = true
                            return
                        }
                        targetIndex = delegateItem.index + 1
                        break

                    default:
                        return
                    }

                    listView.positionViewAtIndex(targetIndex, ListView.Contain)

                    Qt.callLater(function() {
                        let item = listView.itemAtIndex(targetIndex)
                        if (item) {
                            item.requestActiveFocus()
                        }
                    })

                    event.accepted = true
                }

                Connections {
                    target: checkBox.navigation

                    function onActiveChanged() {
                        if (target.active) {
                            listView.positionViewAtIndex(index, ListView.Contain)
                        }
                    }
                }
            }
        }
    }
}
