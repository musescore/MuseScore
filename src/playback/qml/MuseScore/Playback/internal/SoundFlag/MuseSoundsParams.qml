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
import MuseScore.Playback 1.0

Item {
    id: root

    property NavigationPanel navigationPanel: null

    height: !prv.noOptions ? Math.min(flickable.contentHeight, 132) : noOptionsLabel.implicitHeight

    MuseSoundsParamsModel {
        id: museSoundsParamsModel
    }

    Component.onCompleted: {
        museSoundsParamsModel.init()
    }

    QtObject {
        id: prv

        property bool noOptions: museSoundsParamsModel.availablePresets.length === 0
    }

    StyledFlickable {
        id: flickable

        anchors.fill: parent

        contentHeight: gridView.implicitHeight

        GridLayout {
            id: gridView

            width: parent.width

            columns: 2
            rows: Math.ceil(museSoundsParamsModel.availablePresets.length / 2)
            columnSpacing: 4
            rowSpacing: 4

            Repeater {
                id: repeaterPreset

                width: parent.width
                height: parent.height

                model: museSoundsParamsModel.availablePresets

                FlatButton {
                    Layout.preferredWidth: (gridView.width - gridView.rowSpacing) / 2
                    Layout.preferredHeight: implicitHeight

                    text: modelData["name"]

                    accentButton: museSoundsParamsModel.presetCodes.indexOf(modelData["code"]) !== -1

                    navigation.name: "Preset" + index
                    navigation.panel: root.navigationPanel
                    navigation.row: index
                    navigation.column: 1

                    onClicked: {
                        museSoundsParamsModel.togglePreset(modelData["code"], mouse.modifiers & Qt.ControlModifier)
                    }
                }
            }
        }
    }

    StyledTextLabel {
        id: noOptionsLabel

        text: qsTrc("playback", "Sound flag options are not available for this sound.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap

        visible: prv.noOptions
    }
}
