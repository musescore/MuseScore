/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import MuseScore.Playback

StyledPopupView {
    id: root

    property PlaybackToolBarModel playbackModel: null

    contentWidth: contentColumn.implicitWidth
    contentHeight: contentColumn.implicitHeight

    ColumnLayout {
        id: contentColumn
        spacing: 8

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("playback", "Override tempo")
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            spacing: 12

            StyledSlider {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 30

                value: root.playbackModel.tempoMultiplier
                from: 0.1
                to: 3.0
                stepSize: 0.05

                fillBackground: false

                onMoved: {
                    root.playbackModel.tempoMultiplier = value
                }
            }

            IncrementalPropertyControl {
                Layout.preferredWidth: 76
                currentValue: (root.playbackModel.tempoMultiplier * 100).toFixed(decimals)

                maxValue: 300
                minValue: 10
                step: 5
                measureUnitsSymbol: "%"
                decimals: 0

                onValueEdited: function(newValue) {
                    root.playbackModel.tempoMultiplier = newValue / 100
                }
            }
        }
    }
}
