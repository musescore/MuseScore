/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias title: titleLabel.text

    property NavigationPanel navigationPanel: null

    property alias model: repeaterPreset.model
    property var selectionModel: null

    signal toggleParamRequested(string paramCode)

    spacing: 8

    StyledTextLabel {
        id: titleLabel

        width: parent.width

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap
    }

    StyledFlickable {
        id: flickable

        width: parent.width
        height: Math.min(contentHeight, 70)

        contentHeight: gridView.implicitHeight

        GridLayout {
            id: gridView

            width: parent.width

            columns: 2
            rows: Math.ceil(root.model.length / 2)
            columnSpacing: 4
            rowSpacing: 4

            Repeater {
                id: repeaterPreset

                width: parent.width
                height: parent.height

                FlatButton {
                    Layout.preferredWidth: (gridView.width - gridView.rowSpacing) / 2
                    Layout.preferredHeight: implicitHeight

                    text: modelData["name"]

                    accentButton: root.selectionModel.indexOf(modelData["code"]) !== -1

                    navigation.name: "Param" + index
                    navigation.panel: root.navigationPanel
                    navigation.row: index
                    navigation.column: 1

                    onClicked: {
                        root.toggleParamRequested(modelData["code"])
                    }
                }
            }
        }
    }
}
