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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

import "internal"

FocusScope {
    id: root

    signal requestActiveFocus()

    QtObject {
        id: prv

        readonly property int sideMargin: 133
        readonly property int buttonWidth: 134
    }

    NavigationSection {
        id: navSec
        name: "RecentScores"
        enabled: root.visible
        order: 3
        onActiveChanged: {
            if (navSec.active) {
                root.requestActiveFocus()
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

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.requestActiveFocus()
                root.forceActiveFocus()
            }
        }
    }

    Item {
        id: headerItem

        anchors.top: parent.top
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        height: childrenRect.height

        NavigationPanel {
            id: navSearchPanel
            name: "HomeScoresSearch"
            section: navSec
            order: 1
            accessible.name: qsTrc("appshell", "Recent scores")
        }

        StyledTextLabel {
            id: pageTitle

            anchors.left: parent.left
            anchors.right: searchField.left
            anchors.rightMargin: 12

            text: qsTrc("userscores", "Scores")
            font: ui.theme.titleBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        SearchField {
            id: searchField

            anchors.centerIn: parent

            navigation.name: "Scores Search"
            navigation.panel: navSearchPanel
            navigation.order: 1
            accessibleName: qsTrc("appshell", "Recent scores")
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

        anchors.top: headerItem.bottom
        anchors.topMargin: 74
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin - view.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin - view.sideMargin
        anchors.bottom: buttonsPanel.top

        navigation.section: navSec
        navigation.order: 2

        backgroundColor: background.color

        model: SortFilterProxyModel {
            sourceModel: recentScoresModel

            filters: [
                FilterValue {
                    roleName: "title"
                    roleValue: searchField.searchText
                    compareType: CompareType.Contains
                }
            ]
        }

        onAddNewScoreRequested: {
            recentScoresModel.addNewScore()
        }

        onOpenScoreRequested: {
            recentScoresModel.openRecentScore(scorePath)
        }
    }

    Rectangle {
        id: buttonsPanel

        anchors.bottom: parent.bottom

        height: 114
        width: parent.width

        color: ui.theme.popupBackgroundColor

        NavigationPanel {
            id: navBottomPanel
            name: "HomeScoresSearch"
            section: navSec
            direction: NavigationPanel.Horizontal
            order: 3
        }

        Row {
            anchors.right : parent.right
            anchors.rightMargin: prv.sideMargin
            anchors.verticalCenter: parent.verticalCenter

            spacing: 22

            FlatButton {

                navigation.name: "NewScore"
                navigation.panel: navBottomPanel
                navigation.column: 1

                width: prv.buttonWidth
                text: qsTrc("userscores", "New")

                onClicked: {
                    recentScoresModel.addNewScore()
                }
            }

            FlatButton {
                navigation.name: "Open other Score"
                navigation.panel: navBottomPanel
                navigation.column: 2

                width: prv.buttonWidth
                text: qsTrc("userscores", "Open other...")

                onClicked: {
                    recentScoresModel.openScore()
                }
            }
        }
    }
}
