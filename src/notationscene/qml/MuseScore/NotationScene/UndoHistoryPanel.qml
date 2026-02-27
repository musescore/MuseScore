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
    //property var snapshots: []

    ListModel {
        id: pinnedModel
        ListElement { text: "File opened"; index: 0; isRedoable: false }
    }

    ColumnLayout {
        anchors.fill: parent

        StyledListView {
            id: pinnedListView
            //objectName: "Undo/redo pinned history"
            Layout.fillWidth: true
            implicitHeight: contentHeight
            model: pinnedModel

            delegate: ListItemBlank {
                id: pinnedListItem

                required property string text
                required property int index
                readonly property bool isRedoable: index > undoHistoryModel.currentIndex

                isSelected: listView.currentIndex === pinnedListItem.index

                onClicked: {
                    undoHistoryModel.undoRedoToIndex(pinnedListItem.index)
                    listView.highlightMoveDuration = 10
                    ListView.positionViewAtIndex(index, ListView.Beginning)
                    // var snapshot = snapshots[pinnedListItem.index]
                    // undoHistoryModel.restoreSnapshot(snapshot);
                }

                RowLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 6

                    Item {
                        implicitWidth: 16
                        implicitHeight: 16

                        component SampleMenuButton: FlatButton {
                            anchors.centerIn: parent
                            visible: pinnedListItem.index != 0
                            transparent: true
                            onClicked: {
                                var items = [
                                    {id: index, icon: null, title: "Unpin history check-point", enabled: true}
                                ]
                                menuLoader1.toggleOpened(items)
                            }

                            StyledMenuLoader {
                                id: menuLoader1
                                onHandleMenuItem: function(id) {
                                    for (var i = 0; i < pinnedModel.count; ++i) {
                                        if (pinnedModel.get(i).index === index) {
                                            pinnedModel.remove(i);
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        SampleMenuButton {
                            icon: IconCode.MENU_THREE_DOTS
                            mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton
                        }
                    }

                    StyledTextLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft

                        text: pinnedListItem.text
                        font: {
                            if (pinnedListItem.ListView.isCurrentItem) {
                                return ui.theme.bodyBoldFont
                            }
                            if (pinnedListItem.isRedoable) {
                                return Qt.font(Object.assign({}, ui.theme.bodyFont, { italic: true }))
                            }
                            return ui.theme.bodyFont
                        }

                        opacity: pinnedModel.isRedoable ? 0.7 : 1
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
                visible: index != 0
                height: index === 0 ? 0 : implicitHeight
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
                        //implicitHeight: checkMark.implicitHeight

                        component MenuButton : FlatButton {
                            anchors.centerIn: parent
                            visible: listItem.ListView.isCurrentItem
                            transparent: true
                            onClicked: {
                                var items = [
                                    {id: 0, icon: null, title: "Pin history check-point", enabled: true},

                                ]
                                menuLoader.toggleOpened(items)
                            }

                            StyledMenuLoader {
                                id: menuLoader
                                onHandleMenuItem: function(index) {
                                    for (var i = 0; i < pinnedModel.count; ++i) {
                                        if (pinnedModel.get(i).index === listItem.index) {
                                            return
                                        }
                                    }
                                    popup.open()
                                }
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
                                        hint: "Enter check-point name"
                                        width: 200
                                        onTextEdited: function (newText) { currentText = newText }
                                        focus: true
                                        onAccepted: {
                                            pinnedModel.append({
                                                text: textInput.currentText, //listItem.text,
                                                index: listItem.index,
                                                isRedoable: listItem.isRedoable
                                            })
                                            //var snapshot = undoHistoryModel.saveSnapshot()
                                            //console.log("snapshot size: " + snapshots.length)

                                            //snapshots.append(snapshot)

                                            // for (var i = 0; i < index; i++) {
                                            //     snapshot.append(undoHistoryModel.get(i))
                                            // }

                                            // snapshots.push(snapshot)
                                            // pinnedModel.append({
                                            //     text: textInput.currentText, //listItem.text,
                                            //     index: snapshots.length -1 ,
                                            //     isRedoable: listItem.isRedoable
                                            //  })
                                            popup.close()
                                        }
                                        onEscaped: popup.close()

                                    }
                                }
                            }
                        }

                        MenuButton {
                            icon: IconCode.MENU_THREE_DOTS
                            mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton
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
                }
            }
        }
    }
}
