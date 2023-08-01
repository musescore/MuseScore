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

        return root.viewType === ScoresPageModel.List ? listComp : gridComp
    }

    Component {
        id: gridComp

        ScoresView.Grid {
            id: grid

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

            navigation.name: "OnlineScoresGrid"
            navigation.accessible.name: qsTrc("project", "Online scores grid")

            view.footer: cloudScoresModel.state === CloudScoresModel.Loading
                         ? busyIndicatorComp : null

            Component {
                id: busyIndicatorComp

                Item {
                    width: GridView.view ? GridView.view.width : 0
                    height: indicator.implicitHeight + indicator.anchors.topMargin + indicator.anchors.bottomMargin

                    StyledBusyIndicator {
                        id: indicator

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: grid.view.spacingBetweenRows / 2
                        anchors.bottomMargin: grid.view.spacingBetweenRows / 2
                    }
                }
            }
        }
    }

    Component {
        id: listComp

        ScoresView.List {
            id: list

            readonly property int minimumInitialItems: (view.height / view.rowHeight) * 2
            readonly property bool almostAtEnd: view.contentHeight - (view.contentY + view.height) < 20 * view.rowHeight

            Component.onCompleted: {
                updateDesiredRowCount()
            }

            onMinimumInitialItemsChanged: {
                updateDesiredRowCount()
            }

            onAlmostAtEndChanged: {
                updateDesiredRowCount()
            }

            function updateDesiredRowCount() {
                Qt.callLater(function() {
                    cloudScoresModel.desiredRowCount = Math.max(minimumInitialItems,
                                                                almostAtEnd ? cloudScoresModel.rowCount + 20 : cloudScoresModel.rowCount)
                })
            }

            navigation.name: "OnlineScoresList"
            navigation.accessible.name: qsTrc("project", "Online scores list")

            columns: [
                ScoresListView.ColumnItem {
                    header: qsTrc("project/cloud", "Visibility")

                    width: function (parentWidth) {
                        let parentWidthExclusingSpacing = parentWidth - list.columns.length * list.view.columnSpacing;
                        return 0.16 * parentWidthExclusingSpacing
                    }

                    delegate: RowLayout {
                        id: row

                        visible: !label.isEmpty
                        spacing: 8

                        readonly property var iconAndText: {
                            switch (score.cloudVisibility ?? 0) {
                            case CloudVisibility.Private:
                                return { "iconCode": IconCode.LOCK_CLOSED, "text": qsTrc("project/cloud", "Private") }
                            case CloudVisibility.Unlisted:
                                return { "iconCode": IconCode.LOCK_OPEN, "text": qsTrc("project/cloud", "Unlisted") }
                            case CloudVisibility.Public:
                                return { "iconCode": IconCode.GLOBE, "text": qsTrc("project/cloud", "Public") }
                            }
                            return { "iconCode": IconCode.NONE, "text": "" }
                        }

                        StyledIconLabel {
                            iconCode: row.iconAndText.iconCode
                        }

                        StyledTextLabel {
                            id: label
                            Layout.fillWidth: true

                            // TODO: accessibility
                            text: row.iconAndText.text

                            font: ui.theme.largeBodyFont
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                },

                ScoresListView.ColumnItem {
                    //: Stands for "Last time that this score was modified".
                    //: Used as the header of this column in the scores list.
                    header: qsTrc("project", "Modified")

                    width: function (parentWidth) {
                        let parentWidthExclusingSpacing = parentWidth - list.columns.length * list.view.columnSpacing;
                        return 0.16 * parentWidthExclusingSpacing
                    }

                    delegate: StyledTextLabel {
                        // TODO: accessibility
                        text: score.timeSinceModified ?? ""

                        font.capitalization: Font.AllUppercase
                        horizontalAlignment: Text.AlignLeft
                    }
                },

                ScoresListView.ColumnItem {
                    header: qsTrc("project", "Size", "file size")

                    width: function (parentWidth) {
                        let parentWidthExclusingSpacing = parentWidth - list.columns.length * list.view.columnSpacing;
                        return 0.13 * parentWidthExclusingSpacing
                    }

                    delegate: StyledTextLabel {
                        // TODO: accessibility
                        text: score.fileSize ?? ""

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft
                    }
                },

                ScoresListView.ColumnItem {
                    //: Stands for "The number of times this score was viewed on MuseScore.com".
                    //: Used as the header of this column in the scores list.
                    header: qsTrc("project", "Views", "number of views")

                    width: function (parentWidth) {
                        let parentWidthExclusingSpacing = parentWidth - list.columns.length * list.view.columnSpacing;
                        return Math.max(0.08 * parentWidthExclusingSpacing, 76)
                    }

                    delegate: RowLayout {
                        visible: !label.isEmpty
                        spacing: 8

                        StyledIconLabel {
                            iconCode: IconCode.EYE_OPEN
                        }

                        StyledTextLabel {
                            id: label
                            Layout.fillWidth: true

                            // TODO: accessibility
                            text: score.cloudViewCount ?? ""

                            font: ui.theme.largeBodyFont
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            ]

            view.footer: cloudScoresModel.state === CloudScoresModel.Loading
                         ? busyIndicatorComp : null

            Component {
                id: busyIndicatorComp

                Item {
                    width: ListView.view ? ListView.view.width : 0
                    height: list.view.rowHeight

                    StyledBusyIndicator {
                        id: indicator

                        anchors.centerIn: parent
                    }
                }
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

                title: qsTrc("project", "You don't have any online scores yet")
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
                    body: qsTrc("project", "Login or create a new account on <a href=\"https://musescore.com\">musescore.com</a> to view online scores.")
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
                body: qsTrc("project", "Please check your internet connection, or try again later.")
            }
        }
    }
}
