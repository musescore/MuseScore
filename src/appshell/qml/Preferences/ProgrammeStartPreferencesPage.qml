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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    ProgrammeStartPreferencesModel {
        id: programmeStartModel
    }

    Column {
        anchors.fill: parent

        spacing: 20

        StyledTextLabel {
            text: qsTrc("appshell", "Programme Start")
            font: ui.theme.bodyBoldFont
        }

        RadioButtonGroup {
            spacing: 16
            orientation: Qt.Vertical

            width: parent.width

            model: programmeStartModel.startupModes

            delegate: Row {
                spacing: 0

                RoundedRadioButton {
                    anchors.verticalCenter: parent.verticalCenter

                    width: 220

                    checked: modelData.checked
                    text: modelData.title

                    onClicked: {
                        programmeStartModel.setCurrentStartupMode(model.index)
                    }
                }

                FilePicker {
                    width: 240

                    dialogTitle: qsTrc("appshell", "Choose Starting Score")
                    filter: programmeStartModel.scorePathFilter()

                    visible: modelData.canSelectScorePath
                    path: modelData.scorePath

                    onPathEdited: {
                        programmeStartModel.setStartupScorePath(newPath)
                    }
                }
            }
        }

        ListView {
            spacing: 16
            interactive: false

            width: parent.width
            height: contentHeight

            model: programmeStartModel.panels

            delegate: CheckBox {
                text: modelData.title
                checked: modelData.visible

                onClicked: {
                    programmeStartModel.setPanelVisible(model.index, !checked)
                }
            }
        }
    }
}
