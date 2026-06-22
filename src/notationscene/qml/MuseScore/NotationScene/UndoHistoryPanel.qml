/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

import MuseScore.NotationScene

Item {
    id: root

    property var navigationSection: null
    property int navigationOrderStart: 0

    ColumnLayout {
        anchors.fill: parent

        ExpandableBlank {
            title: "Pinned score versions"
            anchors.fill: parent
            anchors.leftMargin: 12

            contentItemComponent: StyledListView {
                id: snapshotListView
                Layout.fillWidth: true
                implicitHeight: contentHeight
                model: undoHistoryModel.snapshots

                delegate: ListItemBlank {
                    id: snapshotListItem
                    height: snapshotText.implicitHeight + 16

                    required property string name
                    required property int index

                    onClicked: undoHistoryModel.restoreSnapshot(index)

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 6

                        StyledTextLabel {
                            id: snapshotText
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                            text: snapshotListItem.name
                            font: ui.theme.bodyFont
                            wrapMode: Text.WordWrap
                        }

                        FlatButton {
                            id: unpinButton
                            Layout.alignment: Qt.AlignRight
                            //visible: index != 0
                            icon: IconCode.STAR
                            toolTipTitle: "Remove snapshot"
                            transparent: true
                            onClicked: undoHistoryModel.removeSnapshot(index)
                        }
                    }
                }
           }
        }

        SeparatorLine {
            id: separator
            Layout.fillWidth: true
        }

        StyledListView {
            id: listView

            objectName: "Undo/redo history"

            Layout.fillWidth: true
            Layout.fillHeight: true

            currentIndex: undoHistoryModel.currentIndex

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart

            model: UndoHistoryModel {
                id: undoHistoryModel
            }

            delegate: ListItemBlank {
                id: listItem

                required property string text
                required property int index
                readonly property bool isRedoable: index > undoHistoryModel.currentIndex

                isSelected: ListView.isCurrentItem
                navigation.panel: listView.navigation
                navigation.order: index
                navigation.accessible.name: text
                navigation.accessible.row: index

                onClicked: {
                    undoHistoryModel.undoRedoToIndex(index)
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

                        text: listItem.text
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

                    FlatButton {
                        id: pinButton
                        Layout.alignment: Qt.AlignRight
                        visible: listItem.ListView.isCurrentItem
                        icon: IconCode.STAR
                        transparent: true
                        toolTipTitle: "Pin a snapshot of current score"
                        onClicked: popup.open()
                    }

                    StyledPopupView {
                        id: popup
                        contentWidth: layout.childrenRect.width
                        contentHeight: layout.childrenRect.height

                        Column {
                            id: layout
                            spacing: 12
                            anchors.fill: parent

                            TextInputField {
                                id: textInput
                                hint: "Enter snapshot name"
                                width: 200
                                onTextEdited: function (newText) { currentText = newText }
                                focus: true

                                onVisibleChanged: {
                                    if (visible) {
                                        ensureActiveFocus()
                                    }
                                }
                                onAccepted: {
                                    undoHistoryModel.addSnapshot(textInput.currentText)
                                    popup.close()
                                }
                                onEscaped: popup.close()
                            }
                        }
                    }
                }
            }
        }
    }
}
