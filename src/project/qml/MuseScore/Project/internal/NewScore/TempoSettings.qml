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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project
import MuseScore.NotationScene

FlatButton {
    id: root

    property AdditionalInfoModel model: null
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

            spacing: 26

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
                Layout.rightMargin: 32

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

            SeparatorLine {}

            RadioButtonGroup {
                id: tempoMarkingView

                Layout.leftMargin: 32
                Layout.rightMargin: 32

                model: root.model.tempoNotes()

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "TempoMarkingPanel"
                    section: popup.navigationSection
                    order: 2
                }

                delegate: FlatRadioButton {
                    id: delegateButton

                    required property string noteSymbol
                    required property int noteIcon
                    required property bool withDot
                    required property int index

                    width: 36
                    height: width

                    enabled: withTempo.checked
                    checked: index === root.model.currentTempoNoteIndex

                    navigation.name: noteSymbol
                    navigation.panel: tempoMarkingView.navigationPanel
                    navigation.row: 1
                    navigation.column: index

                    navigation.accessible.name: root.model.tempoAccessibleName(noteIcon, withDot)

                    onToggled: {
                        var tempo = root.model.tempo
                        tempo.noteIcon = noteIcon
                        tempo.withDot = withDot
                        root.model.tempo = tempo
                    }

                    StyledTextLabel {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: 10
                        font.family: ui.theme.musicalFont.family
                        font.pixelSize: 24
                        font.letterSpacing: 2
                        text: delegateButton.noteSymbol
                    }
                }
            }

            Row {
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 26

                spacing: 6
                enabled: withTempo.checked

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "="
                }

                IncrementalPropertyControl {
                    id: control

                    implicitWidth: 76

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
