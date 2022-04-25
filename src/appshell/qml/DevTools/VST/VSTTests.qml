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
import MuseScore.Ui 1.0
import MuseScore.Vst 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    NavigationSection {
        id: navSec
        name: "VstTests"
        enabled: root.visible
        order: 1
    }

    VstPluginListModelExample {
        id: pluginListModel

        Component.onCompleted: {
            load()
        }
    }

    Row {
        anchors {
            top: parent.top
            topMargin: 24
            left: parent.left
            leftMargin: 24
            right: parent.right
            rightMargin: 24
        }

        NavigationPanel {
            id: dropdownNav
            name: "Dropdowns"
            section: navSec
            order: 1
        }

        Column {

            spacing: 4

            StyledTextLabel {
                id: sequencesTitle

                text: "Track sequence"
            }

            StyledDropdown {
                id: sequences

                navigation.name: "TrackSequence Dropdown"
                navigation.panel: dropdownNav
                navigation.order: 1

                textRole: "value"
                valueRole: "value"

                currentIndex: sequences.indexOfValue(pluginListModel.currentSequenceId)
                model: pluginListModel.sequenceIdList

                onActivated: function(index, value) {
                    pluginListModel.currentSequenceId = value
                }
            }
        }

        Column {

            spacing: 4

            StyledTextLabel {
                id: tracksTitle

                text: "Track"
            }

            StyledDropdown {
                id: tracks

                navigation.name: "Tracks Dropdown"
                navigation.panel: dropdownNav
                navigation.order: 2

                textRole: "value"
                valueRole: "value"

                model: pluginListModel.trackIdList
                currentIndex: tracks.indexOfValue(pluginListModel.currentTrackId)

                onActivated: function(index, value) {
                    pluginListModel.currentTrackId = value
                }
            }
        }

        Column {

            spacing: 4

            StyledTextLabel {
                id: synthResourceTitle

                text: "Synth resource"
            }

            StyledDropdown {
                id: synthResources

                width: 200

                navigation.name: "Synth Resources Dropdown"
                navigation.panel: dropdownNav
                navigation.order: 3

                textRole: "value"
                valueRole: "value"

                model: pluginListModel.availableSynthResources
                currentIndex: synthResources.indexOfValue(pluginListModel.currentSynthResource)

                onActivated: function(index, value) {
                    pluginListModel.currentSynthResource = value
                }
            }
        }

        Column {

            spacing: 4

            StyledTextLabel {
                id: fxResourcesTitle

                text: "Synth resource"
            }

            StyledDropdown {
                id: fxResources

                width: 200

                navigation.name: "Synth Resources Dropdown"
                navigation.panel: dropdownNav
                navigation.order: 3

                textRole: "value"
                valueRole: "value"

                model: pluginListModel.availableFxResources
                currentIndex: fxResources.indexOfValue(pluginListModel.currentFxResource)

                onCurrentValueChanged: {
                    if (fxResources.currentIndex == -1) {
                        return
                    }

                    pluginListModel.currentFxResource = fxResources.currentValue
                }
            }
        }

        FlatButton {
            id: synthEditorButton

            height: parent.height

            text: "Show plugin view"

            onClicked: {
                pluginListModel.showSynthPluginEditor()
            }
        }

        FlatButton {
            id: fxEditorButton

            height: parent.height

            text: "Show plugin view"

            onClicked: {
                pluginListModel.showFxPluginEditor()
            }
        }
    }
}
