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

ColumnLayout {
    id: root

    property QtObject model: null

    height: 400
    width: 400
    spacing: 12

    StyledTabBar {
        id: tabBar
        width: parent.width
        spacing: 12

        StyledTabButton {
            fillWidth: true
            text: /*qsTrc*/ "Single note"
        }

        StyledTabButton {
            fillWidth: true
            text: /*qsTrc*/ "Multi note"
        }
    }

    StackLayout {
        currentIndex: tabBar.currentIndex

        ArticulationPatternsList {
            editorModel: root.model
            model: root.model ? root.model.singleNoteItems : null
        }

        ArticulationPatternsList {
            editorModel: root.model
            model: root.model ? root.model.multiNoteItems : null
        }
    }
}
