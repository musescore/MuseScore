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

import "internal/ScoresPage"

FocusScope {
    id: root

    QtObject {
        id: prv

        readonly property int sideMargin: 46
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

    ScoresPageModel {
        id: scoresPageModel
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
            enabled: topLayout.enabled && topLayout.visible
            section: navSec
            order: 1
            accessible.name: qsTrc("project", "Scores")
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

    StyledTabBar {
        id: tabBar

        anchors.top: topLayout.bottom
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        NavigationPanel {
            id: navTabPanel
            name: "HomeScoresTabs"
            section: navSec
            direction: NavigationPanel.Horizontal
            order: 2
            accessible.name: qsTrc("project", "Scores tab bar")
            enabled: tabBar.enabled && tabBar.visible

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", tabBar.currentItem.navigation.name)
                }
            }
        }

        StyledTabButton {
            text: qsTrc("project", "New & recent")

            navigation.name: "New and recent"
            navigation.panel: navTabPanel
            navigation.column: 1
        }

        StyledTabButton {
            text: qsTrc("project", "My online scores")

            navigation.name: "My online scores"
            navigation.panel: navTabPanel
            navigation.column: 2
        }
    }

    Loader {
        id: contentLoader

        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: buttonsPanel.top

        sourceComponent: [newAndRecentComp, onlineScoresComp][tabBar.currentIndex]
    }

    Component {
        id: newAndRecentComp

        RecentScoresView {
            anchors.fill: parent

            searchText: searchField.searchText

            backgroundColor: background.color
            sideMargin: prv.sideMargin
            topMargin: 24

            navigationSection: navSec
            navigationOrder: 3

            onCreateNewScoreRequested: {
                scoresPageModel.createNewScore()
            }

            onOpenScoreRequested: function(scorePath) {
                scoresPageModel.openScore(scorePath)
            }
        }
    }

    Component {
        id: onlineScoresComp

        Item {
            // TODO
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
                scoresPageModel.openScoreManager()
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

                text: qsTrc("project", "New")

                onClicked: {
                    scoresPageModel.createNewScore()
                }
            }

            FlatButton {
                navigation.name: "Open other Score"
                navigation.panel: navBottomPanel
                navigation.column: 3

                text: qsTrc("project", "Open other…")

                onClicked: {
                    scoresPageModel.openOther()
                }
            }
        }
    }
}
