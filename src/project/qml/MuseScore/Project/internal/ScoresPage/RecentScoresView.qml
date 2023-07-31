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

ScoresView {
    id: root

    model: RecentScoresModel {
        id: recentScoresModel
    }

    Component.onCompleted: {
        recentScoresModel.load()
    }

    sourceComponent: root.viewType === ScoresPageModel.List ? listComp : gridComp

    Component {
        id: gridComp

        ScoresView.Grid {
            navigation.name: "RecentScoresGrid"
            navigation.accessible.name: qsTrc("project", "Recent scores grid")
        }
    }

    Component {
        id: listComp

        ScoresView.List {
            id: list

            showNewScoreItem: true

            navigation.name: "RecentScoresList"
            navigation.accessible.name: qsTrc("project", "Recent scores list")

            columns: [
                ScoresListView.ColumnItem {
                    //: Stands for "Last time that this score was modified".
                    //: Used as the header of this column in the scores list.
                    header: qsTrc("project", "Modified")

                    width: function (parentWidth) {
                        let parentWidthExclusingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.25 * parentWidthExclusingSpacing
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
                        let parentWidthExclusingSpacing = parentWidth - (list.columns.length - 1) * list.view.columnSpacing;
                        return 0.15 * parentWidthExclusingSpacing
                    }

                    delegate: StyledTextLabel {
                        // TODO: accessibility
                        text: score.fileSize ?? ""

                        font.capitalization: Font.AllUppercase
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            ]
        }
    }
}
