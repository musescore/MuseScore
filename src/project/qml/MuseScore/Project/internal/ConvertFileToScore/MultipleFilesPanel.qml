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
    signal removeLastFileRequested()

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
            id: fileListBackground

            Layout.fillWidth: true
            Layout.fillHeight: true

            color: ui.theme.backgroundPrimaryColor
            radius: 3
            clip: true

            StyledListView {
                id: fileListView

                anchors.fill: parent
                anchors.topMargin: 12
                anchors.leftMargin: 12
                anchors.bottomMargin: 12
                anchors.rightMargin: (fileListView.ScrollBar.vertical && fileListView.ScrollBar.vertical.visible) ? 6 : 12

                clip: false

                spacing: 4
                topMargin: root.filesModel.count === 1 ? Math.max(0, (fileListView.height - 40) / 2) : 0

                property int scrollBarGap: 8

                navigation.section: root.navigationPanel.section
                navigation.order: 0
                navigation.direction: NavigationPanel.Both
                accessible.name: qsTrc("project/convert", "Selected files")

                ScrollBar.vertical: StyledScrollBar {
                    thickness: fileListView.scrollBarThickness
                    policy: ScrollBar.AlwaysOn
                }

                model: root.filesModel

                delegate: DropArea {
                    id: dropArea

                    required property int index
                    required property string fileNameRole
                    required property string fileSizeRole

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
                        fileSize: dropArea.fileSizeRole
                        iconCode: root.filesModel.fileIconCode
                        selectable: root.filesModel.count > 1
                        isSelected: root.filesModel.count > 1 && fileListView.currentIndex === dropArea.index

                        navigation.panel: fileListView.navigation
                        navigation.row: dropArea.index
                        navigation.column: 0

                        onNavigationActivated: fileListView.currentIndex = dropArea.index

                        removeButtonNavigation.panel: fileListView.navigation
                        removeButtonNavigation.row: dropArea.index
                        removeButtonNavigation.column: 1

                        onRemoveButtonNavigationActivated: {
                            fileListView.currentIndex = dropArea.index
                            fileListView.positionViewAtIndex(dropArea.index, ListView.Contain)
                        }

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
                        onRemoveSelectionRequested: {
                            if (root.filesModel.count === 1) {
                                root.removeLastFileRequested()
                            } else {
                                root.filesModel.removeAt(dropArea.index)
                            }
                        }

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

            Rectangle {
                anchors.fill: parent

                color: "transparent"
                border.width: 1
                border.color: ui.theme.strokeColor
                radius: fileListBackground.radius
            }
        }

        StyledTextLabel {
            id: combinedFilesNoteLabel

            Layout.fillWidth: true

            text: root.filesModel.combinedFilesNote
            visible: Boolean(combinedFilesNoteLabel.text)
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.alignment: Qt.AlignTop

                spacing: 2

                StyledTextLabel {
                    id: maxCountReachedLabel

                    visible: Boolean(maxCountReachedLabel.text)

                    //: %1 is the number of files currently selected, %2 is the maximum allowed, e.g. "3/5 max files"
                    text: root.filesModel.maxFileCount > 0 && root.filesModel.count > 1
                          ? qsTrc("project/convert", "%1/%2 max files").arg(root.filesModel.count).arg(root.filesModel.maxFileCount)
                          : ""
                    horizontalAlignment: Text.AlignLeft
                    color: ui.theme.fontSecondaryColor
                }

                StyledTextLabel {
                    id: usedSizeLabel

                    visible: Boolean(usedSizeLabel.text)

                    text: root.filesModel.usedSizeLabel
                    horizontalAlignment: Text.AlignLeft
                    color: root.filesModel.exceedsLimits ? "#FA7878" : ui.theme.fontSecondaryColor
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ColumnLayout {
                spacing: 8

                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 0

                    FlatButton {
                        icon: IconCode.ARROW_UP
                        toolTipTitle: qsTrc("global", "Move up")

                        enabled: fileListView.count > 1 && fileListView.currentIndex > 0

                        navigation.panel: root.navigationPanel
                        navigation.order: 1

                        onClicked: root.moveCurrentFile(-1)
                    }

                    FlatButton {
                        Layout.leftMargin: 4

                        icon: IconCode.ARROW_DOWN
                        toolTipTitle: qsTrc("global", "Move down")

                        enabled: fileListView.count > 1 && fileListView.currentIndex >= 0 && fileListView.currentIndex < root.filesModel.count - 1

                        navigation.panel: root.navigationPanel
                        navigation.order: 2

                        onClicked: root.moveCurrentFile(1)
                    }

                    FlatButton {
                        id: selectMoreButton

                        Layout.leftMargin: 12

                        text: qsTrc("global", "Select more")
                        accentButton: true

                        enabled: root.filesModel.maxFileCount <= 0 || root.filesModel.count < root.filesModel.maxFileCount

                        navigation.panel: root.navigationPanel
                        navigation.order: 3

                        onClicked: {
                            root.selectMoreFilesRequested(root.filesModel.paths)
                        }
                    }
                }

                FileRequirements {
                    Layout.alignment: Qt.AlignRight

                    fileRequirements: root.fileRequirements

                    navigation.panel: root.navigationPanel
                    navigation.order: 4
                }
            }
        }
    }
}
