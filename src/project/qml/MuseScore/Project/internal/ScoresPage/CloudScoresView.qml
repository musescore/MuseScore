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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.Cloud 1.0

ScoresView {
    id: root

    model: CloudScoresModel {
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

        return root.viewType === ScoresView.ViewType_List ? listComp : gridComp
    }

    Component {
        id: gridComp

        ScoresView.Grid {
            readonly property int fittingItems: view.columns * (view.rows + 1)
            readonly property bool almostAtEnd: view.contentHeight - (view.contentY + view.height) < 2 * view.cellHeight

            Component.onCompleted: {
                updateDesiredRowCount()
            }

            onFittingItemsChanged: {
                updateDesiredRowCount()
            }

            onAlmostAtEndChanged: {
                updateDesiredRowCount()
            }

            function updateDesiredRowCount() {
                Qt.callLater(function() {
                    cloudScoresModel.desiredRowCount = Math.max(fittingItems,
                                                                almostAtEnd ? cloudScoresModel.rowCount + view.columns : cloudScoresModel.rowCount)
                })
            }
        }
    }

    Component {
        id: listComp

        ScoresView.List {}
    }

    component Message : Column {
        property alias title: titleLabel.text
        property alias body: bodyLabel.text

        spacing: 16

        StyledTextLabel {
            id: titleLabel
            width: parent.width
            font: ui.theme.tabBoldFont
        }

        StyledTextLabel {
            id: bodyLabel
            width: parent.width
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

                title: qsTrc("project", "You don't have any online scores yet")
                body: qsTrc("project", `Scores will appear here when you save a file to the cloud, or publish a score on <a href="https://musescore.com">musescore.com</a>.`)
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
                    body: qsTrc("project", `Login or create a new account on <a href="https://musescore.com">musescore.com</a> to view online scores.`)
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
                        accessible.name: qsTrc("appshell", "Sign in buttons")
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
                body: qsTrc("project", "Please check your internet connection, or try again later.")
            }
        }
    }
}
