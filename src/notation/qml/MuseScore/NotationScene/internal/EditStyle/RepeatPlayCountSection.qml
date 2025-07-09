/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    id: root
    anchors.fill: parent;
    color: ui.theme.backgroundPrimaryColor

    RepeatPlayCountTextModel {
        id: repeatPlayCountTextModel
    }

    signal goToTextStylePage(string s)

    ColumnLayout {
        width: parent.width
        spacing: 12

        RowLayout {
            ToggleButton {
                checked: repeatPlayCountTextModel.repeatPlayCountShow.value === true
                onToggled: {
                    repeatPlayCountTextModel.repeatPlayCountShow.value = !repeatPlayCountTextModel.repeatPlayCountShow.value
                }
            }

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
                text: qsTrc("notation/editstyle/repeatplaycount", "Automatically show text at repeat barlines")
            }
        }

        CheckBox {
            text: qsTrc("notation/editstyle/repeatplaycount", "Show for single repeats (e.g. “x2”)")
            checked: repeatPlayCountTextModel.repeatPlayCountShowSingleRepeats.value === true
            onClicked: repeatPlayCountTextModel.repeatPlayCountShowSingleRepeats.value = !repeatPlayCountTextModel.repeatPlayCountShowSingleRepeats.value

            enabled: repeatPlayCountTextModel.repeatPlayCountShow.value === true
        }

        StyledTextLabel {
            text: qsTrc("notation", "Preset")
            enabled: repeatPlayCountTextModel.repeatPlayCountShow.value === true
        }

        RowLayout {
            spacing: 6
            Layout.fillWidth: true

            enabled: repeatPlayCountTextModel.repeatPlayCountShow.value === true

            ComboBoxDropdown {
                Layout.preferredWidth: 196
                model: repeatPlayCountTextModel.textPresetOptions()

                styleItem: repeatPlayCountTextModel.repeatTextPreset
            }

            FlatButton {
                text: qsTrc("notation", "Edit text style")

                onClicked: {
                    root.goToTextStylePage("repeat-play-count")
                }
            }
        }

    }
}
