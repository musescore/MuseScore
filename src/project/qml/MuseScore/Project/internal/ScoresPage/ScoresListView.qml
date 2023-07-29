/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Item {
    id: root

    property AbstractScoresModel model
    property var columns: []
    property alias showNewScoreItem: newScoreItem.visible
    property string searchText

    property color backgroundColor: ui.theme.backgroundSecondaryColor
    property real sideMargin: 46

    property alias view: view

    property alias navigation: navPanel

    signal createNewScoreRequested()
    signal openScoreRequested(var scorePath, var displayName)

    QtObject {
        id: prv

        readonly property real itemInset: 12
        readonly property real rowHeight: 64
        readonly property real columnSpacing: 44
    }

    SortFilterProxyModel {
        id: searchFilterModel
        sourceModel: root.model

        excludeIndexes: root.model.nonScoreItemIndices

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
            implicitHeight: prv.rowHeight

            visible: false
            itemInset: prv.itemInset
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
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                id: listViewColumn

                anchors.fill: parent
                spacing: 0

                // Column headers
                RowLayout {
                    Layout.preferredHeight: 44
                    Layout.leftMargin: prv.itemInset
                    Layout.rightMargin: prv.itemInset

                    spacing: prv.columnSpacing

                    StyledTextLabel {
                        Layout.fillWidth: true

                        text: qsTrc("project", "Name")

                        font: ui.theme.bodyBoldFont
                        Component.onCompleted: { font.capitalization = Font.AllUppercase }

                        horizontalAlignment: Text.AlignLeft
                    }

                    Repeater {
                        model: root.columns

                        delegate: StyledTextLabel {
                            Layout.preferredWidth: modelData.width(parent.width)

                            text: modelData.header

                            font: ui.theme.bodyBoldFont
                            Component.onCompleted: { font.capitalization = Font.AllUppercase }

                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }

                SeparatorLine {}

                StyledListView {
                    id: view

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    bottomMargin: bottomGradient.height

                    readonly property real rowHeight: prv.rowHeight

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

                        itemInset: prv.itemInset
                        implicitHeight: prv.rowHeight
                        columnSpacing: prv.columnSpacing

                        navigation.panel: navPanel
                        navigation.row: index + 1
                        navigation.column: 0
                        navigation.onActiveChanged: {
                            if (navigation.active) {
                                view.positionViewAtIndex(index, ListView.Contain)
                            }
                        }

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
    }
}
