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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

FocusableItem {
    id: root

    property alias noteHeadGroupsModel: gridView.model
    property bool isIndeterminate: false

    function iconByNoteHeadGroup(noteHeadGroup) {
        switch (noteHeadGroup) {
        case NoteHead.HEAD_NORMAL: return "qrc:/resources/icons/notehead_normal.svg"
        case NoteHead.HEAD_CROSS: return "qrc:/resources/icons/notehead_cross.svg"
        case NoteHead.HEAD_PLUS: return "qrc:/resources/icons/notehead_plus.svg"
        case NoteHead.HEAD_XCIRCLE: return "qrc:/resources/icons/notehead_xcircle.svg"
        case NoteHead.HEAD_WITHX: return "qrc:/resources/icons/notehead_xellipse.svg"
        case NoteHead.HEAD_TRIANGLE_UP: return "qrc:/resources/icons/notehead_triangle_down.svg"
        case NoteHead.HEAD_TRIANGLE_DOWN: return "qrc:/resources/icons/notehead_triangle_up.svg"
        case NoteHead.HEAD_SLASHED1: return "qrc:/resources/icons/notehead_slashed_left.svg"
        case NoteHead.HEAD_SLASHED2: return "qrc:/resources/icons/notehead_slashed_right.svg"
        case NoteHead.HEAD_DIAMOND: return "qrc:/resources/icons/notehead_diamond.svg"
        case NoteHead.HEAD_DIAMOND_OLD: return "qrc:/resources/icons/notehead_diamond_old.svg"
        case NoteHead.HEAD_CIRCLED: return "qrc:/resources/icons/notehead_circled.svg"
        case NoteHead.HEAD_CIRCLED_LARGE: return "qrc:/resources/icons/notehead_circled.svg"
        case NoteHead.HEAD_LARGE_ARROW: return "qrc:/resources/icons/notehead_large_arrow.svg"
        case NoteHead.HEAD_BREVIS_ALT: return "qrc:/resources/icons/notehead_brevis_alt.svg"
        case NoteHead.HEAD_SLASH: return "qrc:/resources/icons/notehead_slash.svg"
        case NoteHead.HEAD_SOL: return "qrc:/resources/icons/notehead_sol.svg"
        case NoteHead.HEAD_LA: return "qrc:/resources/icons/notehead_la.svg"
        case NoteHead.HEAD_FA: return "qrc:/resources/icons/notehead_fa.svg"
        case NoteHead.HEAD_MI: return "qrc:/resources/icons/notehead_mi.svg"
        case NoteHead.HEAD_DO: return "qrc:/resources/icons/notehead_do.svg"
        case NoteHead.HEAD_RE: return "qrc:/resources/icons/notehead_re.svg"
        case NoteHead.HEAD_TI: return "qrc:/resources/icons/notehead_ti.svg"
        default: return "qrc:/resources/icons/note.svg"
        }
    }

    width: parent.width
    height: gridView.implicitHeight + 2 * gridView.anchors.margins

    Rectangle {
        id: gridViewEnclosure
        anchors.fill: parent

        color: ui.theme.textFieldColor
        radius: 3
    }

    GridView {
        id: gridView
        anchors.fill: parent
        anchors.margins: 8

        implicitHeight: Math.min(contentHeight, 3 * cellHeight)

        cellHeight: 40
        cellWidth: 40

        interactive: true
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {}

        delegate: FocusableItem {
            implicitHeight: gridView.cellHeight
            implicitWidth: gridView.cellWidth

            Icon {
                anchors.centerIn: parent

                pixelSize: 30
                icon: iconByNoteHeadGroup(headGroupRole)
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    noteHeadGroupsModel.selectedHeadGroupIndex = index
                }
            }
        }

        highlight: Rectangle {
            color: ui.theme.accentColor
            opacity: ui.theme.accentOpacityNormal
            radius: 2
        }

        currentIndex: noteHeadGroupsModel && !isIndeterminate ? noteHeadGroupsModel.selectedHeadGroupIndex : -1
    }
}
