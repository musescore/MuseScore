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

pragma ComponentBehavior: Bound

import QtQuick

import MuseScore.NotationScene
import MuseScore.Project

Item {
    id: root

    property alias name: notationView.name
    readonly property alias navigationSection: notationView.navigationSection

    NotationView {
        id: notationView

        anchors.fill: parent

        readOnly: true
    }

    ReviewScoreQualityPanel {
        id: reviewPanel

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12

        z: 100

        navigationSection: root.navigationSection
        navigationOrderStart: notationView.navigationOrderEnd + 1

        onCloseRequested: reviewPanel.visible = false
    }
}
