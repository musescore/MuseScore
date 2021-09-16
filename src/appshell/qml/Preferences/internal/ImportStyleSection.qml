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

import MuseScore.UiComponents 1.0

BaseSection {
    id: root

    title: qsTrc("appshell", "Style used for import")

    property string styleFileImportPath: ""
    property string fileChooseTitle: ""
    property string filePathFilter: ""
    property string fileDirectory: ""

    signal styleFileImportPathChangeRequested(string path)

    RoundedRadioButton {
        anchors.left: parent.left
        anchors.right: parent.right

        checked: root.styleFileImportPath === ""

        StyledTextLabel {
            text: qsTrc("appshell", "Built-in style")
            horizontalAlignment: Text.AlignLeft
        }

        onToggled: {
            root.styleFileImportPathChangeRequested("")
        }
    }

    RoundedRadioButton {
        anchors.left: parent.left
        anchors.right: parent.right

        checked: root.styleFileImportPath !== ""

        onToggled: {
            root.styleFileImportPathChangeRequested("")
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

                dialogTitle: root.fileChooseTitle
                filter: root.filePathFilter
                dir: root.fileDirectory

                path: root.styleFileImportPath

                onPathEdited: {
                    root.styleFileImportPathChangeRequested(newPath)
                }
            }
        }
    }
}
