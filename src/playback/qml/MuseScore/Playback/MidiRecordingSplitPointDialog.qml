/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

StyledDialogView {
    id: root

    title: qsTrc("playback", "Split point")

    contentWidth: 360
    contentHeight: contentColumn.implicitHeight + 48

    margins: 24

    MidiRecordingSplitPointModel {
        id: splitPointModel
    }

    Component.onCompleted: {
        splitPointModel.load()
    }

    onNavigationActivateRequested: {
        pitchDropdown.navigation.requestActive()
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent
        spacing: 16

        StyledTextLabel {
            Layout.fillWidth: true

            text: qsTrc("playback", "When recording on two staves, the notes equal to or higher than the split point go to the selected staff, the lower notes to the staff below.")
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "SplitPointPanel"
            section: root.navigationSection
            direction: NavigationPanel.Horizontal
            order: 1
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            StyledDropdown {
                id: pitchDropdown

                Layout.preferredWidth: 160

                model: splitPointModel.pitches
                currentIndex: indexOfValue(splitPointModel.splitPitch)

                enabled: !splitPointModel.isLearning

                navigation.name: "SplitPitchDropdown"
                navigation.panel: contentColumn.navigationPanel
                navigation.order: 1

                onActivated: function(index, value) {
                    splitPointModel.splitPitch = value
                }
            }

            FlatButton {
                Layout.fillWidth: true

                text: splitPointModel.isLearning ? qsTrc("playback", "Press a key… (click to cancel)")
                                                 : qsTrc("playback", "Learn from keyboard")
                accentButton: splitPointModel.isLearning

                navigation.name: "LearnFromKeyboardButton"
                navigation.panel: contentColumn.navigationPanel
                navigation.order: 2

                onClicked: {
                    splitPointModel.toggleLearning()
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "SplitPointButtonsPanel"
                section: root.navigationSection
                direction: NavigationPanel.Horizontal
                order: 2
            }

            FlatButton {
                width: 100
                text: qsTrc("global", "Cancel")

                navigation.panel: parent.navigationPanel
                navigation.order: 1

                onClicked: {
                    splitPointModel.cancel()
                    root.hide()
                }
            }

            FlatButton {
                width: 100
                text: qsTrc("global", "OK")
                accentButton: true

                navigation.panel: parent.navigationPanel
                navigation.order: 2

                onClicked: {
                    splitPointModel.apply()
                    root.hide()
                }
            }
        }
    }
}
