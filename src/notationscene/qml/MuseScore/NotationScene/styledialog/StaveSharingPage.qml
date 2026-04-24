/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick.Layouts
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyledFlickable {
    id: root

    signal goToTextStylePage(int index)

    contentWidth: Math.max(column.implicitWidth, root.width)
    contentHeight: column.implicitHeight

    StaveSharingPageModel {
        id: staveSharingModel
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 12

        ColumnLayout {
            width: parent.width
            spacing: 6

            StyleToggle {
                id: enableStaveSharingToggle
                Layout.fillWidth: true
                styleItem: staveSharingModel.enableStaveSharing
                text: qsTrc("notation/editstyle/stavesharing", "Enable stave sharing")
            }
        }
    }
}
