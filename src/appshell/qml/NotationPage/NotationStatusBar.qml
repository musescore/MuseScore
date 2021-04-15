/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Workspace 1.0

Rectangle {
    id: root

    width: parent.width
    height: 36

    NotationAccessibilityInfo {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: statusBarRow.left
        anchors.verticalCenter: parent.verticalCenter

        horizontalAlignment: Text.AlignLeft
    }

    Row {
        id: statusBarRow

        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter

        spacing: 12

        WorkspacesControl {
            anchors.verticalCenter: parent.verticalCenter
        }

        ConcertPitchControl {
            anchors.verticalCenter: parent.verticalCenter
        }

        ViewModeControl {
            anchors.verticalCenter: parent.verticalCenter
        }

        ZoomControl {
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
