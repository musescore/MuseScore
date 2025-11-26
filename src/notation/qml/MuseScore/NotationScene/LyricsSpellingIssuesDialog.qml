/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
import MuseScore.NotationScene 1.0

StyledDialogView {
    id: root

    title: qsTrc("notation", "Lyrics spelling issues")
    contentWidth: 500
    contentHeight: 400
    modal: false
    resizable: true
    alwaysOnTop: true

    LyricsSpellingIssuesModel {
        id: issuesModel
    }

    Component.onCompleted: {
        issuesModel.load()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Header with stats
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            StyledTextLabel {
                text: issuesModel.issueCount === 0
                      ? qsTrc("notation", "No spelling issues found")
                      : qsTrc("notation", "%n issue(s) found", "", issuesModel.issueCount)
                font: ui.theme.bodyBoldFont
            }

            Item { Layout.fillWidth: true }

            StyledTextLabel {
                text: qsTrc("notation", "Language: %1").arg(issuesModel.language)
                visible: issuesModel.language.length > 0
            }
        }

        // Issues list
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ui.theme.backgroundSecondaryColor
            radius: 4
            border.color: ui.theme.strokeColor
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
                spacing: 0

                // Column headers
                Rectangle {
                    Layout.fillWidth: true
                    height: 32
                    color: ui.theme.backgroundQuarternaryColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        StyledTextLabel {
                            Layout.preferredWidth: 150
                            text: qsTrc("notation", "Word")
                            font: ui.theme.bodyBoldFont
                            horizontalAlignment: Text.AlignLeft
                        }

                        StyledTextLabel {
                            Layout.preferredWidth: 60
                            text: qsTrc("notation", "Count")
                            font: ui.theme.bodyBoldFont
                            horizontalAlignment: Text.AlignCenter
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: qsTrc("notation", "First location")
                            font: ui.theme.bodyBoldFont
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }

                // List view
                StyledListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: issuesModel

                    delegate: ListItemBlank {
                        id: listItem
                        width: ListView.view ? ListView.view.width : 0
                        height: 36

                        isSelected: ListView.isCurrentItem

                        onClicked: {
                            listView.currentIndex = model.index
                        }

                        onDoubleClicked: {
                            issuesModel.goToIssue(model.index)
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            StyledTextLabel {
                                Layout.preferredWidth: 150
                                text: model.word
                                horizontalAlignment: Text.AlignLeft
                                elide: Text.ElideRight
                            }

                            StyledTextLabel {
                                Layout.preferredWidth: 60
                                text: model.count
                                horizontalAlignment: Text.AlignCenter
                            }

                            StyledTextLabel {
                                Layout.fillWidth: true
                                text: model.location
                                horizontalAlignment: Text.AlignLeft
                                elide: Text.ElideRight
                            }
                        }
                    }

                    ScrollBar.vertical: StyledScrollBar {}
                }
            }

            // Empty state
            StyledTextLabel {
                anchors.centerIn: parent
                visible: issuesModel.issueCount === 0 && issuesModel.isAvailable
                text: qsTrc("notation", "All lyrics are spelled correctly!")
                font: ui.theme.largeBodyFont
            }
        }

        // Button row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            FlatButton {
                text: qsTrc("notation", "Rescan")
                onClicked: {
                    issuesModel.rescan()
                }
            }

            Item { Layout.fillWidth: true }

            FlatButton {
                text: qsTrc("notation", "Go to")
                enabled: listView.currentIndex >= 0
                onClicked: {
                    issuesModel.goToIssue(listView.currentIndex)
                }
            }

            FlatButton {
                text: qsTrc("global", "Close")
                onClicked: {
                    root.hide()
                }
            }
        }
    }
}
