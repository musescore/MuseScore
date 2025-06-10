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
    anchors.fill: parent
    color: ui.theme.backgroundPrimaryColor

    RepeatBarlinesSectionModel {
        id: repeatBarlinesSectionModel
    }

    ColumnLayout {
        width: parent.width
        spacing: 12

        RowLayout {
            id: beforeSigChangesToggle

            ToggleButton {
                id: toggleButton
                checked: repeatBarlinesSectionModel.barlineBeforeSigChange.value === true
                onToggled: {
                    repeatBarlinesSectionModel.barlineBeforeSigChange.value = !repeatBarlinesSectionModel.barlineBeforeSigChange.value
                }
            }

            StyledTextLabel {
                id : toggleText
                text: qsTrc("notation/editstyle/barlines", "Show barline before key and time signature changes")
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignLeft
            }
        }

        CheckBox {
            id: beforeKeysigBox
            text: qsTrc("notation/editstyle/barlines", "Use double barline before key signatures")
            enabled: repeatBarlinesSectionModel.barlineBeforeSigChange.value === true
            checked: repeatBarlinesSectionModel.doubleBarlineBeforeKeySig.value === true
            onClicked: repeatBarlinesSectionModel.doubleBarlineBeforeKeySig.value = !repeatBarlinesSectionModel.doubleBarlineBeforeKeySig.value
        }

        CheckBox {
            id: beforeTimesigBox
            text: qsTrc("notation/editstyle/barlines", "Use double barline before time signatures")
            enabled: repeatBarlinesSectionModel.barlineBeforeSigChange.value === true
            checked: repeatBarlinesSectionModel.doubleBarlineBeforeTimeSig.value === true
            onClicked: repeatBarlinesSectionModel.doubleBarlineBeforeTimeSig.value = !repeatBarlinesSectionModel.doubleBarlineBeforeTimeSig.value
        }
    }
}
