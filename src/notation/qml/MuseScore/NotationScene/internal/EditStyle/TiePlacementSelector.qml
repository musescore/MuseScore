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

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

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

        StyledTextLabel {
            text: qsTrc("notation", "Placement of inner ties with respect to augmentation dots:")
        }


        RoundedRadioButton {
            id: autoButton
            width: parent.width
            checked: tiePlacementSelector.placementDots.value === 0
            onToggled: tiePlacementSelector.placementDots.value = 0
            text: qsTrc("notation", "Auto")
        }

        RoundedRadioButton {
            id: beforeButton
            width: parent.width
            checked: tiePlacementSelector.placementDots.value === 1
            onToggled: tiePlacementSelector.placementDots.value = 1
            text: qsTrc("notation", "Always before dots")
        }

        RoundedRadioButton {
            id: afterButton
            width: parent.width
            checked: tiePlacementSelector.placementDots.value === 2
            onToggled: tiePlacementSelector.placementDots.value = 2
            text: qsTrc("notation", "Always after dots")
        }
    }
}

