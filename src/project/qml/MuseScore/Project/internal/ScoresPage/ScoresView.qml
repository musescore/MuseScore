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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Project 1.0

Loader {
    id: root

    property AbstractScoresModel model
    property string searchText

    property int viewType: ScoresView.ViewType_Grid

    property color backgroundColor: ui.theme.backgroundSecondaryColor
    property real sideMargin: 46

    property NavigationSection navigationSection
    property int navigationOrder

    signal createNewScoreRequested()
    signal openScoreRequested(var scorePath, var displayName)

    enum ViewType {
        ViewType_Grid,
        ViewType_List
    }

    component Grid : ScoresGridView {
        anchors.fill: parent

        model: root.model
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

    component List : StyledTextLabel {
        anchors.centerIn: parent
        text: "Not yet implemented"
    }
}
