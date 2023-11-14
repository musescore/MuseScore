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

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    TiePlacementSelectorModel {
        id: tiePlacementSelector
    }

    Column {
        spacing: 8

        StyledTextLabel {
            text: qsTrc("notation", "Placement on single notes:")
            width: parent.width
            horizontalAlignment: Text.AlignLeft
        }

        RadioButtonGroup {
            height: 30
            model: [
                { iconCode: IconCode.TIE_INSIDE, value: 1, title: qsTrc("inspector", "Inside") },
                { iconCode: IconCode.TIE_OUTSIDE, value: 2, title: qsTrc("inspector", "Outside") }
            ]

            delegate: FlatRadioButton {
                width: 106
                height: 30

                checked: modelData.value === tiePlacementSelector.placementSingleNotes.value
                iconCode: modelData.iconCode
                navigation.accessible.name: modelData.title

                onToggled: {
                    tiePlacementSelector.placementSingleNotes.value = modelData.value
                }
            }
        }

        StyledTextLabel {
            text: qsTrc("notation", "Placement on chords:")
        }

        RadioButtonGroup {
            model: [
                { iconCode: IconCode.TIE_CHORD_INSIDE, value: 1, title: qsTrc("inspector", "Inside") },
                { iconCode: IconCode.TIE_CHORD_OUTSIDE, value: 2, title: qsTrc("inspector", "Outside") }
            ]

            delegate: FlatRadioButton {
                width: 106
                height: 30

                checked: modelData.value === tiePlacementSelector.placementChords.value
                iconCode: modelData.iconCode
                navigation.accessible.name: modelData.title

                onToggled: {
                    tiePlacementSelector.placementChords.value = modelData.value
                }
            }
        }
    }
}

