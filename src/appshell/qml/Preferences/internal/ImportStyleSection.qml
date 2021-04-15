/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Column {
    spacing: 18

    property var preferencesModel: null

    StyledTextLabel {
        text: qsTrc("appshell", "Style Used for Import")
        font: ui.theme.bodyBoldFont
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 12

        RoundedRadioButton {
            anchors.left: parent.left
            anchors.right: parent.right

            checked: preferencesModel.styleFileImportPath === ""

            StyledTextLabel {
                text: qsTrc("appshell", "Built-in style")
                horizontalAlignment: Text.AlignLeft
            }

            onToggled: {
                preferencesModel.styleFileImportPath = ""
            }
        }

        RoundedRadioButton {
            anchors.left: parent.left
            anchors.right: parent.right

            checked: preferencesModel.styleFileImportPath !== ""

            onToggled: {
                preferencesModel.styleFileImportPath = ""
            }

            Item {
                StyledTextLabel {
                    id: title

                    width: 193
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTrc("appshell", "Use style file:")
                    horizontalAlignment: Text.AlignLeft
                }

                FilePicker {
                    anchors.left: title.right
                    width: 246
                    anchors.verticalCenter: parent.verticalCenter

                    dialogTitle: preferencesModel.styleChooseTitle()
                    filter: preferencesModel.stylePathFilter()
                    dir: preferencesModel.fileDirectory(preferencesModel.styleFileImportPath)

                    path: preferencesModel.styleFileImportPath

                    onPathEdited: {
                        preferencesModel.styleFileImportPath = newPath
                    }
                }
            }
        }
    }
}
