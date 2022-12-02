/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

import "internal"

FocusScope {
    id: root

    QtObject {
        id: prv

        readonly property int sideMargin: 46
        readonly property int buttonWidth: 134
    }

    NavigationSection {
        id: navSec
        name: "RecentScores"
        enabled: root.enabled && root.visible
        order: 3
        onActiveChanged: {
            if (navSec.active) {
                root.forceActiveFocus()
            }
        }
    }

    RecentScoresModel {
        id: recentScoresModel
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor
    }

    RowLayout {
        id: topLayout

        anchors.top: parent.top
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        spacing: 12

        NavigationPanel {
            id: navSearchPanel
            name: "HomeScoresSearch"
            section: navSec
            order: 1
            accessible.name: qsTrc("project", "Recent scores")
        }

        StyledTextLabel {
            id: pageTitle
            Layout.fillWidth: true

            text: qsTrc("project", "Scores")
            font: ui.theme.titleBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "Scores Search"
            navigation.panel: navSearchPanel
            navigation.order: 1
            accessible.name: qsTrc("project", "Search recent scores")
        }
    }

    Rectangle {
        anchors.top: view.top

        width: parent.width
        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: background.color
            }

            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    RecentScoresView {
        id: view

        anchors.top: topLayout.bottom
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin - view.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin - view.sideMargin
        anchors.bottom: buttonsPanel.top

        navigation.section: navSec
        navigation.order: 2

        backgroundColor: background.color

        isSearching: searchField.searchText.length != 0

        model: SortFilterProxyModel {
            sourceModel: recentScoresModel

            excludeIndexes: [0, recentScoresModel.rowCount() - 1]           // New score and no result items

            filters: [
                FilterValue {
                    roleName: "name"
                    roleValue: searchField.searchText
                    compareType: CompareType.Contains
                }
            ]
        }

        onAddNewScoreRequested: {
            recentScoresModel.addNewScore()
        }

        onOpenScoreRequested: function(scorePath) {
            recentScoresModel.openRecentScore(scorePath)
        }
    }

    Rectangle {
        anchors.bottom: buttonsPanel.top

        width: parent.width
        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }

            GradientStop {
                position: 1.0
                color: buttonsPanel.color
            }
        }
    }

    Rectangle {
        id: buttonsPanel

        anchors.bottom: parent.bottom

        height: 100
        width: parent.width

        color: ui.theme.backgroundSecondaryColor

        NavigationPanel {
            id: navBottomPanel
            name: "RecentScoresBottom"
            section: navSec
            direction: NavigationPanel.Horizontal
            order: 3

            //: accessibility name for the panel at the bottom of the "Scores" page
            accessible.name: qsTrc("project", "Scores actions")
        }

        FlatButton {
            anchors.left: parent.left
            anchors.leftMargin: prv.sideMargin
            anchors.verticalCenter: parent.verticalCenter

            navigation.name: "ScoreManager"
            navigation.panel: navBottomPanel
            navigation.column: 1

            minWidth: 216
            text: qsTrc("project", "Score manager (online)")

            onClicked: {
                recentScoresModel.openScoreManager()
            }
        }

        Row {
            anchors.right : parent.right
            anchors.rightMargin: prv.sideMargin
            anchors.verticalCenter: parent.verticalCenter

            spacing: 22

            FlatButton {

                navigation.name: "NewScore"
                navigation.panel: navBottomPanel
                navigation.column: 2

                minWidth: prv.buttonWidth
                text: qsTrc("project", "New")

                onClicked: {
                    recentScoresModel.addNewScore()
                }
            }

            FlatButton {
                navigation.name: "Open other Score"
                navigation.panel: navBottomPanel
                navigation.column: 3

                minWidth: prv.buttonWidth
                text: qsTrc("project", "Open other…")

                onClicked: {
                    recentScoresModel.openScore()
                }
            }
        }
    }
}
