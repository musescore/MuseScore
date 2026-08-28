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
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: root

    property NavigationPanel navigationPanel: null
    property var filesModel: null
    property var fileRequirements: []

    signal selectMoreFilesRequested(var existingPaths)

    function focusOnFileList() {
        selectMoreButton.navigation.requestActive()
    }

    function moveCurrentFile(delta) {
        var index = fileListView.currentIndex
        root.filesModel.move(index, index + delta)
        fileListView.currentIndex = index + delta
    }

    color: ui.theme.backgroundSecondaryColor
    border.width: 1
    border.color: ui.theme.strokeColor
    radius: 3

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 18

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: ui.theme.backgroundPrimaryColor
            border.width: 1
            border.color: ui.theme.strokeColor
            radius: 3

            StyledListView {
                id: fileListView

                anchors.fill: parent
                anchors.topMargin: 12
                anchors.leftMargin: 12
                anchors.bottomMargin: 12
                anchors.rightMargin: (fileListView.ScrollBar.vertical && fileListView.ScrollBar.vertical.visible) ? 6 : 12

                spacing: 4
                topMargin: root.filesModel.count === 1 ? Math.max(0, (fileListView.height - 40) / 2) : 0

                property int scrollBarGap: 8

                ScrollBar.vertical: StyledScrollBar {
                    thickness: fileListView.scrollBarThickness
                    policy: ScrollBar.AlwaysOn
                }

                model: root.filesModel

                delegate: DropArea {
                    id: dropArea

                    required property int index
                    required property string fileNameRole

                    width: {
                        if (!ListView.view) {
                            return 0
                        }

                        var scrollBar = fileListView.ScrollBar.vertical
                        if (!scrollBar.visible) {
                            return ListView.view.width
                        }

                        //! NOTE: scrollBar.width includes its own leftPadding, which is invisible
                        //! hit-area rather than a visible gap, so it doesn't count towards scrollBarGap
                        var scrollBarHandleLeft = scrollBar.width - scrollBar.leftPadding
                        return ListView.view.width - scrollBarHandleLeft - fileListView.scrollBarGap
                    }
                    height: fileItem.height

                    keys: [ "SelectedFilesListItem" ]

                    onEntered: function(drag) {
                        var draggedItem = drag.source
                        if (!draggedItem || draggedItem.visualIndex === dropArea.index) {
                            return
                        }

                        root.filesModel.move(draggedItem.visualIndex, dropArea.index)
                    }

                    FileItem {
                        id: fileItem

                        //! NOTE: exposed so a dragged sibling's drop target can read it back via Drag.source
                        property int visualIndex: dropArea.index
                        property bool dragged: fileItem.mouseArea.drag.active

                        anchors.verticalCenter: fileItem.dragged ? undefined : dropArea.verticalCenter
                        anchors.horizontalCenter: fileItem.dragged ? undefined : dropArea.horizontalCenter

                        width: dropArea.width
                        z: fileItem.dragged ? 100 : 1

                        fileName: dropArea.fileNameRole
                        iconCode: root.filesModel.fileIconCode
                        selectable: root.filesModel.count > 1
                        isSelected: root.filesModel.count > 1 && fileListView.currentIndex === dropArea.index

                        navigation.panel: fileListView.navigation
                        navigation.order: dropArea.index

                        Drag.active: fileItem.dragged
                        Drag.source: fileItem
                        Drag.keys: [ "SelectedFilesListItem" ]
                        Drag.hotSpot.x: fileItem.width / 2
                        Drag.hotSpot.y: fileItem.height / 2

                        mouseArea.preventStealing: true
                        mouseArea.drag.target: root.filesModel.count > 1 ? fileItem : null
                        mouseArea.drag.axis: Drag.YAxis
                        mouseArea.onReleased: fileItem.Drag.drop()

                        onClicked: fileListView.currentIndex = dropArea.index
                        onRemoveSelectionRequested: root.filesModel.removeAt(dropArea.index)

                        onDraggedChanged: {
                            if (fileItem.dragged) {
                                var pos = fileItem.mapToItem(dropArea.ListView.view.contentItem, 0, 0)
                                fileItem.parent = dropArea.ListView.view.contentItem
                                fileItem.x = pos.x
                                fileItem.y = pos.y
                            } else {
                                fileItem.parent = dropArea
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            FileRequirements {
                fileRequirements: root.fileRequirements

                navigation.panel: root.navigationPanel
                navigation.order: 1
            }

            Item {
                Layout.fillWidth: true
            }

            RowLayout {
                spacing: 8

                FlatButton {
                    icon: IconCode.ARROW_UP

                    enabled: fileListView.count > 1 && fileListView.currentIndex > 0

                    navigation.panel: root.navigationPanel
                    navigation.order: 2

                    onClicked: root.moveCurrentFile(-1)
                }

                FlatButton {
                    icon: IconCode.ARROW_DOWN

                    enabled: fileListView.count > 1 && fileListView.currentIndex >= 0 && fileListView.currentIndex < root.filesModel.count - 1

                    navigation.panel: root.navigationPanel
                    navigation.order: 3

                    onClicked: root.moveCurrentFile(1)
                }

                FlatButton {
                    id: selectMoreButton

                    Layout.leftMargin: 4

                    text: qsTrc("global", "Select more")
                    accentButton: true

                    navigation.panel: root.navigationPanel
                    navigation.order: 4

                    onClicked: {
                        root.selectMoreFilesRequested(root.filesModel.paths())
                    }
                }
            }
        }
    }
}
