/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

FlatRadioButtonGroupPropertyView {
    id: root
    titleText: qsTrc("inspector", "Notehead duration")

    radioButtonGroup.height: 40
    requestIconFontSize: 30

    model: [
        { text: qsTrc("inspector", "Auto"), value: NoteHead.TYPE_AUTO, title: qsTrc("inspector", "Auto") },
        { iconCode: IconCode.NOTE_HEAD_QUARTER, value: NoteHead.TYPE_QUARTER, title: qsTrc("inspector", "Quarter") },
        { iconCode: IconCode.NOTE_HEAD_HALF, value: NoteHead.TYPE_HALF, title: qsTrc("inspector", "Half") },
        { iconCode: IconCode.NOTE_HEAD_WHOLE, value: NoteHead.TYPE_WHOLE, title: qsTrc("inspector", "Whole") },
        { iconCode: IconCode.NOTE_HEAD_BREVIS, value: NoteHead.TYPE_BREVIS, title: qsTrc("inspector", "Brevis") }
    ]
}
