/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.Playback

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 1

    PracticeEmphasisPanelModel {
        id: emphasisModel

        Component.onCompleted: emphasisModel.load()
    }

    NavigationPanel {
        id: navPanel
        name: "PracticeEmphasisPanel"
        section: root.navigationSection
        order: root.contentNavigationPanelOrderStart + 1
        direction: NavigationPanel.Vertical
        accessible.name: qsTrc("playback", "Practice emphasis")
    }

    StyledFlickable {
        id: flickable

        anchors.fill: parent
        anchors.margins: 12

        contentWidth: width
        contentHeight: content.implicitHeight
        clip: true

        PracticeEmphasisContent {
            id: content
            width: flickable.width
            emphasisModel: emphasisModel
            navigationSection: root.navigationSection
            tabNavigationOrder: root.contentNavigationPanelOrderStart
            navigationPanel: navPanel
            showTitle: false
        }
    }
}
