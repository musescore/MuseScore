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
import QtQuick.Controls 1.5
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

TabPanel {
    id: root

    property QtObject patternModel: null

    height: 64
    width: 400

    Tab {
        title: qsTrc("mpe", "Arrangement")

        ArrangementPatternControl {
            anchors.top: parent.top
            anchors.topMargin: 24

            height: implicitHeight
            width: root.width

            patternModel: root.patternModel
        }
    }
    Tab {
        title: qsTrc("mpe", "Pitch")

        PitchPatternControl {
            anchors.top: parent.top
            anchors.topMargin: 24

            height: implicitHeight
            width: root.width

            patternModel: root.patternModel
        }
    }

    Tab {
        title: qsTrc("mpe", "Expression")

        ExpressionPatternControl {
            anchors.top: parent.top
            anchors.topMargin: 24

            height: implicitHeight
            width: root.width

            patternModel: root.patternModel
        }
    }
}
