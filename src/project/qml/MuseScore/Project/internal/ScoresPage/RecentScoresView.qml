/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

ScoresView {
    id: root

    RecentScoresModel {
        id: recentScoresModel
    }

    Component.onCompleted: {
        recentScoresModel.load()
    }

    sourceComponent: root.viewType === ScoresPageModel.List ? listComp : gridComp

    Component {
        id: gridComp

        ScoresGridView {
            anchors.fill: parent

            model: recentScoresModel
            searchText: root.searchText

            backgroundColor: root.backgroundColor
            sideMargin: root.sideMargin

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrder
            navigation.name: "RecentScoresGrid"
            navigation.accessible.name: qsTrc("project", "Recent scores grid")

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

        ScoresListView {
            id: list

            anchors.fill: parent

            model: recentScoresModel
            searchText: root.searchText

            backgroundColor: root.backgroundColor
            sideMargin: root.sideMargin

            showNewScoreItem: true

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrder
            navigation.name: "RecentScoresList"
            navigation.accessible.name: qsTrc("project", "Recent scores list")

            onCreateNewScoreRequested: {
                root.createNewScoreRequested()
            }

            onOpenScoreRequested: function(scorePath, displayName) {
                root.openScoreRequested(scorePath, displayName)
            }

            columns: [
                ScoresListView.ColumnItem {
                    id: composerColumn

                    header: qsTrc("project", "Composer")
                    width: function (parentWidth) {
                        let parentWidthExcludingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.15 * parentWidthExcludingSpacing
                    }

                    delegate: StyledTextLabel {
                        id: composerLabel
                        text: (score.composer && score.composer !== "Composer / arranger") ? score.composer : "-"

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft

                        NavigationFocusBorder {
                            navigationCtrl: NavigationControl {
                                name: "ComposerLabel"
                                panel: navigationPanel
                                row: navigationRow
                                column: navigationColumnStart
                                enabled: composerLabel.visible && composerLabel.enabled && !composerLabel.isEmpty
                                accessible.name: composerColumn.header + ": " + composerLabel.text
                                accessible.role: MUAccessible.StaticText

                                onActiveChanged: {
                                    if (active) {
                                        listItem.scrollIntoView()
                                    }
                                }
                            }

                            anchors.margins: -radius
                            radius: 2 + border.width
                        }
                    }
                },

                ScoresListView.ColumnItem {
                    id: modifiedColumn

                    //: Stands for "Last time that this score was modified".
                    //: Used as the header of this column in the scores list.
                    header: qsTrc("project", "Modified")

                    width: function (parentWidth) {
                        let parentWidthExcludingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.10 * parentWidthExcludingSpacing
                    }

                    delegate: StyledTextLabel {
                        id: modifiedLabel
                        text: score.timeSinceModified ?? ""

                        font.capitalization: Font.AllUppercase
                        horizontalAlignment: Text.AlignLeft

                        NavigationFocusBorder {
                            navigationCtrl: NavigationControl {
                                name: "ModifiedLabel"
                                panel: navigationPanel
                                row: navigationRow
                                column: navigationColumnStart + 3
                                enabled: modifiedLabel.visible && modifiedLabel.enabled && !modifiedLabel.isEmpty
                                accessible.name: modifiedColumn.header + ": " + modifiedLabel.text
                                accessible.role: MUAccessible.StaticText

                                onActiveChanged: {
                                    if (active) {
                                        listItem.scrollIntoView()
                                    }
                                }
                            }

                            anchors.margins: -radius
                            radius: 2 + border.width
                        }
                    }
                },

                ScoresListView.ColumnItem {
                    id: creationDateColumn

                    header: qsTrc("project", "Created")
                    width: function (parentWidth) {
                        let parentWidthExcludingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.10 * parentWidthExcludingSpacing
                    }

                    delegate: StyledTextLabel {
                        id: creationDateLabel
                        text: score.creationDate ?? "-"

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft

                        NavigationFocusBorder {
                            navigationCtrl: NavigationControl {
                                name: "CreationDateLabel"
                                panel: navigationPanel
                                row: navigationRow
                                column: navigationColumnStart + 2
                                enabled: creationDateLabel.visible && creationDateLabel.enabled && !creationDateLabel.isEmpty
                                accessible.name: creationDateColumn.header + ": " + creationDateLabel.text
                                accessible.role: MUAccessible.StaticText

                                onActiveChanged: {
                                    if (active) {
                                        listItem.scrollIntoView()
                                    }
                                }
                            }

                            anchors.margins: -radius
                            radius: 2 + border.width
                        }
                    }
                },

                ScoresListView.ColumnItem {
                    id: filePathColumn

                    header: qsTrc("project", "File Path")
                    width: function (parentWidth) {
                        let parentWidthExcludingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.35 * parentWidthExcludingSpacing
                    }

                    delegate: StyledTextLabel {
                        id: filePathLabel
                        text: score.path ?? "-"

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideMiddle

                        NavigationFocusBorder {
                            navigationCtrl: NavigationControl {
                                name: "FilePathLabel"
                                panel: navigationPanel
                                row: navigationRow
                                column: navigationColumnStart + 4
                                enabled: filePathLabel.visible && filePathLabel.enabled && !filePathLabel.isEmpty
                                accessible.name: filePathColumn.header + ": " + filePathLabel.text
                                accessible.role: MUAccessible.StaticText

                                onActiveChanged: {
                                    if (active) {
                                        listItem.scrollIntoView()
                                    }
                                }
                            }

                            anchors.margins: -radius
                            radius: 2 + border.width
                        }
                    }
                },

                ScoresListView.ColumnItem {
                    id: sizeColumn
                    header: qsTrc("global", "Size", "file size")

                    width: function (parentWidth) {
                        let parentWidthExcludingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.05 * parentWidthExcludingSpacing
                    }

                    delegate: StyledTextLabel {
                        id: sizeLabel
                        text: Boolean(score.fileSize) ? score.fileSize : "-"

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft

                        NavigationFocusBorder {
                            navigationCtrl: NavigationControl {
                                name: "SizeLabel"
                                panel: navigationPanel
                                row: navigationRow
                                column: navigationColumnStart
                                enabled: sizeLabel.visible && sizeLabel.enabled && !sizeLabel.isEmpty
                                accessible.name: sizeColumn.header + ": " + (Boolean(score.fileSize) ? score.fileSize : qsTrc("global", "Unknown"))
                                accessible.role: MUAccessible.StaticText

                                onActiveChanged: {
                                    if (active) {
                                        listItem.scrollIntoView()
                                    }
                                }
                            }

                            anchors.margins: -radius
                            radius: 2 + border.width
                        }
                    }
                }
            ]
        }
    }
}
