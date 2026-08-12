/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
import QtQuick

import Muse.UiComponents
import Muse.Ui
import MuseScore.PropertiesPanel

import "../../../common"

FlatRadioButtonGroupPropertyView {
    id: root
    titleText: qsTrc("propertiespanel", "Notehead duration")

    radioButtonGroup.height: 40
    requestIconFontSize: 30

    model: [
        { text: qsTrc("propertiespanel", "Auto"), value: NoteHead.TYPE_AUTO, title: qsTrc("propertiespanel", "Auto") },
        { iconCode: IconCode.NOTE_HEAD_QUARTER, value: NoteHead.TYPE_QUARTER, title: qsTrc("propertiespanel", "Quarter") },
        { iconCode: IconCode.NOTE_HEAD_HALF, value: NoteHead.TYPE_HALF, title: qsTrc("propertiespanel", "Half") },
        { iconCode: IconCode.NOTE_HEAD_WHOLE, value: NoteHead.TYPE_WHOLE, title: qsTrc("propertiespanel", "Whole") },
        { iconCode: IconCode.NOTE_HEAD_BREVIS, value: NoteHead.TYPE_BREVIS, title: qsTrc("propertiespanel", "Brevis") }
    ]
}
