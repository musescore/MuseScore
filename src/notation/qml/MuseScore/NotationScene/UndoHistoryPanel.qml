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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import MuseScore.NotationScene

Item {
    id: root

    property alias navigationSection: navPanel.section
    property alias navigationOrderStart: navPanel.order

    NavigationPanel {
        id: navPanel
        name: "Undo/redo history"
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
    }

    StyledListView {
        id: listView
        anchors.fill: parent

        model: UndoHistoryModel {
            id: undoHistoryModel
        }

        currentIndex: undoHistoryModel.currentIndex

        delegate: ListItemBlank {
            id: listItem
            isSelected: ListView.isCurrentItem

            readonly property bool isRedoable: model.index > undoHistoryModel.currentIndex

            navigation.panel: navPanel
            navigation.order: model.index
            navigation.accessible.name: model.text
            navigation.accessible.row: model.index

            onClicked: {
                undoHistoryModel.undoRedoToIndex(model.index)
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                spacing: 6

                Item {
                    implicitWidth: 16
                    implicitHeight: checkMark.implicitHeight

                    StyledIconLabel {
                        id: checkMark
                        anchors.centerIn: parent
                        iconCode: IconCode.TICK_RIGHT_ANGLE
                        visible: listItem.ListView.isCurrentItem
                    }
                }

                StyledTextLabel {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft

                    text: model.text
                    font: {
                        if (listItem.ListView.isCurrentItem) {
                            return ui.theme.bodyBoldFont
                        }

                        if (listItem.isRedoable) {
                            return Qt.font(Object.assign({}, ui.theme.bodyFont, { italic: true }))
                        }

                        return ui.theme.bodyFont
                    }

                    opacity: listItem.isRedoable ? 0.7 : 1
                }
            }
        }
    }
}
