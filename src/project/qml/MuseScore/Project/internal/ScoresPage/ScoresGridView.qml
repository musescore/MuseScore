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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Item {
    id: root

    property AbstractScoresModel model
    property string searchText

    property color backgroundColor: ui.theme.backgroundSecondaryColor
    property real sideMargin: 46

    property alias view: view

    property alias navigation: navPanel

    signal createNewScoreRequested()
    signal openScoreRequested(var scorePath, var displayName)

    clip: true

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
        name: "ScoresGridView"
        direction: NavigationPanel.Both
        accessible.name: qsTrc("project", "Scores grid")
    }

    Rectangle {
        id: topGradient

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.backgroundColor
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    StyledGridView {
        id: view

        anchors.fill: parent
        anchors.topMargin: -spacingBetweenRows / 2
        anchors.leftMargin: root.sideMargin - spacingBetweenColumns / 2
        anchors.rightMargin: root.sideMargin - spacingBetweenColumns / 2
        anchors.bottomMargin: -spacingBetweenRows / 2

        topMargin: topGradient.height
        bottomMargin: bottomGradient.height

        readonly property int columns: Math.max(0, Math.floor(width / cellWidth))
        readonly property int rows: Math.max(0, Math.ceil(height / cellHeight))

        readonly property real spacingBetweenColumns: 60
        readonly property real spacingBetweenRows: 40

        readonly property real actualCellWidth: 172
        readonly property real actualCellHeight: 294

        cellWidth: actualCellWidth + spacingBetweenColumns
        cellHeight: actualCellHeight + spacingBetweenRows

        flickableDirection: Flickable.VerticalFlick

        ScrollBar.vertical: StyledScrollBar {
            parent: root

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            visible: view.contentHeight > view.height
            z: 2
        }

        model: searchFilterModel

        delegate: Item {
            width: view.cellWidth
            height: view.cellHeight

            // TODO: when an item is invisible, there is still visual space allocated for it
            visible: score.isNoResultFound ? view.count === root.model.nonScoreItemIndices.length && Boolean(root.searchText) : true

            ScoreGridItem {
                anchors.centerIn: parent

                width: view.actualCellWidth
                height: view.actualCellHeight

                navigation.panel: navPanel
                navigation.row: view.columns === 0 ? 0 : Math.floor(model.index / view.columns)
                navigation.column: (model.index - (navigation.row * view.columns)) * 3 // * 3 because of controls inside ScoreItem
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.positionViewAtIndex(index, ListView.Contain)
                    }
                }

                name: score.name
                path: score.path ?? ""
                suffix: score.suffix ?? ""
                thumbnailUrl: score.thumbnailUrl ?? ""
                isCreateNew: score.isCreateNew
                isNoResultFound: score.isNoResultFound
                isCloud: score.isCloud
                cloudScoreId: score.scoreId ?? 0
                timeSinceModified: score.timeSinceModified ?? ""

                onClicked: {
                    if (isCreateNew) {
                        root.createNewScoreRequested()
                    } else if (!isNoResultFound) {
                        root.openScoreRequested(score.path, score.name)
                    }
                }
            }
        }
    }

    Rectangle {
        id: bottomGradient
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

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
