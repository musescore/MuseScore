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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0
import Muse.Cloud 1.0

import "internal"

ScoresView {
    id: root

    CloudScoresModel {
        id: cloudScoresModel
    }

    Component.onCompleted: {
        cloudScoresModel.load()
    }

    function refresh() {
        cloudScoresModel.reload()
    }

    sourceComponent: {
        switch (cloudScoresModel.state) {
        case CloudScoresModel.NotSignedIn:
            return notSignedInComp
        case CloudScoresModel.Error:
            return errorComp
        case CloudScoresModel.Fine:
        case CloudScoresModel.Loading:
            break;
        }

        if (cloudScoresModel.rowCount == 0 && !cloudScoresModel.hasMore && cloudScoresModel.state != CloudScoresModel.Loading) {
            return emptyComp
        }

        return root.viewType === ScoresPageModel.List ? listComp : gridComp
    }

    Component {
        id: gridComp

        CloudScoresGridView {
            anchors.fill: parent

            model: cloudScoresModel
            searchText: root.searchText

            backgroundColor: root.backgroundColor
            sideMargin: root.sideMargin

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrder

            onCreateNewScoreRequested: {
                root.createNewScoreRequested()
            }

            onOpenScoreRequested: function(scorePath, displayName) {
                root.openScoreRequested(scorePath, displayName)
            }
        }
    }

    Component {
        id: listComp

        CloudScoresListView {
            anchors.fill: parent

            model: cloudScoresModel
            searchText: root.searchText

            backgroundColor: root.backgroundColor
            sideMargin: root.sideMargin

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrder

            onCreateNewScoreRequested: {
                root.createNewScoreRequested()
            }

            onOpenScoreRequested: function(scorePath, displayName) {
                root.openScoreRequested(scorePath, displayName)
            }
        }
    }

    Component {
        id: emptyComp

        Item {
            anchors.fill: parent

            Message {
                anchors.top: parent.top
                anchors.topMargin: Math.max(parent.height / 3 - height / 2, 0)
                anchors.left: parent.left
                anchors.leftMargin: root.sideMargin
                anchors.right: parent.right
                anchors.rightMargin: root.sideMargin

                title: qsTrc("project", "You don’t have any online scores yet")
                body: qsTrc("project", "Scores will appear here when you save a file to the cloud, or publish a score on <a href=\"https://musescore.com\">musescore.com</a>.")
            }
        }
    }

    Component {
        id: notSignedInComp

        Item {
            anchors.fill: parent

            Column {
                anchors.top: parent.top
                anchors.topMargin: Math.max(parent.height / 3 - height / 2, 0)
                anchors.left: parent.left
                anchors.leftMargin: root.sideMargin
                anchors.right: parent.right
                anchors.rightMargin: root.sideMargin

                spacing: 32

                Message {
                    width: parent.width

                    title: qsTrc("project", "You are not signed in")
                    body: qsTrc("project", "Log in or create a new account on <a href=\"https://musescore.com\">musescore.com</a> to view online scores.")
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: implicitWidth
                    spacing: 12

                    MuseScoreComAuthorizationModel {
                        id: authorizationModel
                    }

                    Component.onCompleted: {
                        authorizationModel.load()
                    }

                    NavigationPanel {
                        id: navPanel
                        name: "SignInButtons"
                        section: root.navigationSection
                        order: root.navigationOrder
                        direction: NavigationPanel.Horizontal
                        accessible.name: qsTrc("cloud", "Sign in buttons")
                    }

                    FlatButton {
                        navigation.panel: navPanel
                        navigation.order: 1

                        text: qsTrc("cloud", "Create account")
                        onClicked: {
                            authorizationModel.createAccount()
                        }
                    }

                    FlatButton {
                        navigation.panel: navPanel
                        navigation.order: 2

                        text: qsTrc("cloud", "Sign in")
                        onClicked: {
                            authorizationModel.signIn()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: errorComp

        Item {
            anchors.fill: parent

            Message {
                anchors.top: parent.top
                anchors.topMargin: Math.max(parent.height / 3 - height / 2, 0)
                anchors.left: parent.left
                anchors.leftMargin: root.sideMargin
                anchors.right: parent.right
                anchors.rightMargin: root.sideMargin

                title: qsTrc("project", "Unable to load online scores")
                body: qsTrc("global", "Please check your internet connection or try again later.")
            }
        }
    }
}
