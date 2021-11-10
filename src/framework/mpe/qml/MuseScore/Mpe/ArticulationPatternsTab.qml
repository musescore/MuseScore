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

    property QtObject model: null

    height: 400
    width: 400

    Tab {
        title: qsTrc("mpe", "Single Note")

        ArticulationPatternsList {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            editorModel: root.model
            model: root.model ? root.model.singleNoteItems : 0
        }
    }
    Tab {
        title: qsTrc("mpe", "Multi Note")

        ArticulationPatternsList {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            editorModel: root.model
            model: root.model ? root.model.multiNoteItems : 0
        }
    }
}
