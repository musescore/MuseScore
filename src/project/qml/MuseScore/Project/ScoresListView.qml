/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/ScoresPage"

Item {
    id: root

    property AbstractScoresModel model
    property list<ColumnItem> columns
    property alias showNewScoreItem: newScoreItem.visible
    property string searchText

    property color backgroundColor: ui.theme.backgroundSecondaryColor
    property real sideMargin: 46

    property alias view: view

    property alias navigation: navPanel

    signal createNewScoreRequested()
    signal openScoreRequested(var scorePath, var displayName)

    component ColumnItem : QtObject {
        property string header

        property var width: function (parentWidth) {
            return parentWidth / 5
        }

        property Component delegate
    }

    // Component for sortable headers
    component SortableHeader : Rectangle {
        property string headerText
        property string sortKey
        property bool sortable: true
        
        height: 44
        color: "transparent"
        
        Row {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4
            
            StyledTextLabel {
                text: headerText
                font: Qt.font(Object.assign({}, ui.theme.bodyBoldFont, { capitalization: Font.AllUppercase }))
                horizontalAlignment: Text.AlignLeft
                anchors.verticalCenter: parent.verticalCenter

                color: headerMouseArea.containsMouse ? "grey" : "white"
            }
            
            StyledIconLabel {
                visible: sortable && root.model && root.model.currentSortKey === sortKey
                iconCode: (root.model && root.model.sortOrder === Qt.AscendingOrder) ? IconCode.ARROW_UP : IconCode.ARROW_DOWN
                font.pixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        MouseArea {
            id: headerMouseArea
            anchors.fill: parent
            hoverEnabled: sortable
            cursorShape: sortable ? Qt.PointingHandCursor : Qt.ArrowCursor
            
            onClicked: {
                if (sortable && root.model && root.model.sortBy) {
                    root.model.sortBy(sortKey)
                }
            }
        }
    }

    SortFilterProxyModel {
        id: searchFilterModel
        sourceModel: root.model

        alwaysExcludeIndices: root.model ? root.model.nonScoreItemIndices : []

        filters: [
            FilterValue {
                roleName: "name"
                roleValue: root.searchText
                compareType: CompareType.Contains
            }
        ]
    }

    NavigationPanel {
        id: navPanel
        name: "ScoresListView"
        direction: NavigationPanel.Both
        accessible.name: qsTrc("project", "Scores list")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: sideMargin
        anchors.rightMargin: sideMargin

        spacing: 12

        ScoreListItem {
            id: newScoreItem

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            implicitHeight: view.rowHeight

            visible: false
            itemInset: view.itemInset
            showBottomBorder: false

            navigation.panel: navPanel
            navigation.row: 0
            navigation.column: 0

            score: {
                "name": qsTrc("project", "New score")
            }

            thumbnailComponent: Rectangle {
                anchors.fill: parent
                color: "white"

                StyledIconLabel {
                    anchors.centerIn: parent

                    iconCode: IconCode.PLUS

                    font.pixelSize: 16
                    color: "black"
                }
            }

            onClicked: root.createNewScoreRequested()
        }

        Item {
            id: listViewContainer

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: view.count > 0 || view.header || view.footer

            ColumnLayout {
                id: listViewColumn

                anchors.fill: parent
                spacing: 0

                // Column headers with sorting functionality
                RowLayout {
                    Layout.preferredHeight: 44
                    Layout.leftMargin: view.itemInset
                    Layout.rightMargin: view.itemInset

                    spacing: view.columnSpacing

                    SortableHeader {
                        Layout.fillWidth: true
                        headerText: qsTrc("project", "Name")
                        sortKey: "name"
                    }

                    Repeater {
                        model: root.columns

                        delegate: SortableHeader {
                            Layout.preferredWidth: modelData.width(parent.width)
                            
                            headerText: modelData.header
                            sortKey: {
                                // Map header text to sort keys
                                if (modelData.header === qsTrc("project", "Composer")) return "composer"
                                if (modelData.header === qsTrc("project", "Modified")) return "timeSinceModified"
                                if (modelData.header === qsTrc("project", "Created")) return "creationDate"
                                if (modelData.header === qsTrc("project", "File Path")) return "path"
                                if (modelData.header === qsTrc("global", "Size", "file size")) return "fileSize"
                                if (modelData.header === qsTrc("project", "Views", "number of views")) return "cloudViewCount"
                                if (modelData.header === qsTrc("project/cloud", "Visibility")) return "cloudVisibility"
                                return ""
                            }
                            sortable: sortKey !== ""
                        }
                    }
                }

                SeparatorLine {}

                StyledListView {
                    id: view

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    bottomMargin: bottomGradient.height

                    readonly property real itemInset: 12
                    readonly property real rowHeight: 64
                    readonly property real columnSpacing: 44

                    ScrollBar.vertical: StyledScrollBar {
                        parent: root

                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right

                        visible: view.contentHeight > view.height
                        z: 2
                    }

                    model: searchFilterModel

                    delegate: ScoreListItem {
                        required property int index

                        columns: root.columns

                        itemInset: view.itemInset
                        implicitHeight: view.rowHeight
                        columnSpacing: view.columnSpacing

                        navigation.panel: navPanel
                        navigation.row: index + 1
                        navigation.column: 0

                        onClicked: {
                            root.openScoreRequested(score.path, score.name)
                        }
                    }
                }
            }

            Rectangle {
                id: bottomGradient
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                visible: listViewColumn.visible

                height: 8
                z: 1

                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "transparent"
                    }

                    GradientStop {
                        position: 1.0
                        color: root.backgroundColor
                    }
                }
            }
        }

        Item {
            id: noResultsMessage

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: Boolean(root.searchText) && !listViewContainer.visible

            Message {
                anchors.top: parent.top
                anchors.topMargin: Math.max(parent.height / 3 - height / 2, 0)
                anchors.left: parent.left
                anchors.leftMargin: root.sideMargin
                anchors.right: parent.right
                anchors.rightMargin: root.sideMargin

                title: qsTrc("global", "No results found")
            }
        }
    }
}
