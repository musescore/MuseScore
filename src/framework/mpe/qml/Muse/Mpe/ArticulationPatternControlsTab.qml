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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    property QtObject patternModel: null

    height: 64
    width: 400
    spacing: 12

    StyledTabBar {
        id: tabBar
        width: parent.width
        spacing: 12

        StyledTabButton {
            fillWidth: true
            text: /*qsTrc*/ "Arrangement"
        }

        StyledTabButton {
            fillWidth: true
            text: /*qsTrc*/ "Pitch"
        }

        StyledTabButton {
            fillWidth: true
            text: /*qsTrc*/ "Expression"
        }
    }

    StackLayout {
        width: parent.width
        currentIndex: tabBar.currentIndex

        height: itemAt(currentIndex).implicitHeight

        ArrangementPatternControl {
            height: implicitHeight

            patternModel: root.patternModel
        }

        PitchPatternControl {
            height: implicitHeight

            patternModel: root.patternModel
        }

        ExpressionPatternControl {
            height: implicitHeight

            patternModel: root.patternModel
        }
    }
}
