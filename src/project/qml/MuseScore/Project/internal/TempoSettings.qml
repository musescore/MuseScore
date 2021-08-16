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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.CommonScene 1.0

FlatButton {
    id: root

    property var model: null

    height: 96
    accentButton: popup.visible

    TempoView {
        anchors.centerIn: parent

        noteSymbol: root.model.tempo.noteSymbol
        tempoValue: root.model.tempo.value

        noteSymbolFont.pixelSize: 36
        tempoValueFont: ui.theme.headerFont

        noteSymbolTopPadding: 22
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopup {
        id: popup

        implicitHeight: 250
        implicitWidth: 520

        x: root.x - (width - root.width) / 2
        y: root.height

        Column {
            anchors.fill: parent
            anchors.margins: 10

            spacing: 30

            CheckBox {
                id: withTempo

                anchors.left: parent.left
                anchors.right: parent.right

                checked: root.model.withTempo

                text: qsTrc("project", "Show tempo marking on my score")

                onClicked: {
                    root.model.withTempo = !checked
                }
            }

            SeparatorLine {
                anchors.leftMargin: -(parent.anchors.leftMargin + popup.leftPadding)
                anchors.rightMargin: -(parent.anchors.rightMargin + popup.rightPadding)
            }

            RadioButtonGroup {
                anchors.horizontalCenter: parent.horizontalCenter

                height: 50

                model: root.model.tempoNotes()

                clip: true
                boundsBehavior: ListView.StopAtBounds

                delegate: FlatRadioButton {
                    width: 48
                    height: width

                    enabled: withTempo.checked
                    checked: model.index === root.model.currentTempoNoteIndex

                    onClicked: {
                        var tempo = root.model.tempo
                        tempo.noteIcon = modelData.noteIcon
                        tempo.withDot = modelData.withDot
                        root.model.tempo = tempo
                    }

                    StyledTextLabel {
                        topPadding: 24
                        font.family: ui.theme.musicalFont.family
                        font.pixelSize: 24
                        font.letterSpacing: 1
                        lineHeightMode: Text.FixedHeight
                        lineHeight: 10
                        text: modelData.noteSymbol
                    }
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 20
                enabled: withTempo.checked

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "="
                    font: ui.theme.headerFont
                }

                IncrementalPropertyControl {
                    id: control

                    implicitWidth: 126

                    currentValue: root.model.tempo.value
                    step: 1
                    decimals: 0
                    maxValue: root.model.tempoValueRange().max
                    minValue: root.model.tempoValueRange().min

                    onValueEdited: {
                        var tempo = root.model.tempo
                        tempo.value = newValue
                        root.model.tempo = tempo
                    }
                }
            }
        }
    }
}
