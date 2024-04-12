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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.CommonScene 1.0

FlatButton {
    id: root

    property var model: null
    property string currentValueAccessibleName: model.tempoAccessibleName(root.model.tempo.noteIcon,
                                                                          root.model.tempo.withDot) + " " + root.model.tempo.value

    property alias popupAnchorItem: popup.anchorItem

    height: 96
    accentButton: popup.isOpened

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

    StyledPopupView {
        id: popup

        margins: 0

        contentWidth: content.width
        contentHeight: content.height

        onOpened: {
            withTempo.navigation.requestActive()
        }

        ColumnLayout {
            id: content

            spacing: 0

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "TempoSettingsPanel"
                section: popup.navigationSection
                direction: NavigationPanel.Both
                order: 1
            }

            CheckBox {
                id: withTempo

                Layout.topMargin: 26
                Layout.leftMargin: 32

                checked: root.model.withTempo

                text: qsTrc("project/newscore", "Show tempo marking on my score")

                navigation.name: "WithTempoBox"
                navigation.panel: NavigationPanel {
                    name: "WithTempoBoxPanel"
                    section: popup.navigationSection
                    order: 1
                }
                navigation.row: 0

                onClicked: {
                    root.model.withTempo = !checked
                }
            }

            SeparatorLine {
                Layout.topMargin: 26
            }

            RadioButtonGroup {
                id: tempoMarkingView

                Layout.topMargin: 26
                Layout.leftMargin: 32
                Layout.rightMargin: 32

                height: 48

                model: root.model.tempoNotes()

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "TempoMarkingPanel"
                    section: popup.navigationSection
                    order: 2
                }

                delegate: FlatRadioButton {
                    width: 48
                    height: width

                    enabled: withTempo.checked
                    checked: model.index === root.model.currentTempoNoteIndex

                    navigation.name: modelData.noteSymbol
                    navigation.panel: tempoMarkingView.navigationPanel
                    navigation.row: 1
                    navigation.column: model.index

                    navigation.accessible.name: root.model.tempoAccessibleName(modelData.noteIcon, modelData.withDot)

                    onToggled: {
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
                Layout.topMargin: 26
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 22

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

                    navigation.panel: NavigationPanel {
                        name: "TempoPanel"
                        section: popup.navigationSection
                        order: 3
                    }
                    navigation.row: 0

                    onValueEdited: function(newValue) {
                        var tempo = root.model.tempo
                        tempo.value = newValue
                        root.model.tempo = tempo
                    }
                }
            }
        }
    }
}
