/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 1

    anchors.fill: parent

    property Component toolbarComponent: PercussionPanelToolBar {
        navigation.section: root.navigationSection
        navigation.order: root.contentNavigationPanelOrderStart

        model: percussionPanelModel

        panelWidth: root.width
    }

    PercussionPanelModel {
        id: percussionPanelModel
    }

    Column {
        anchors.fill: parent

        StyledTextLabel {
            // Placeholder demonstrating main states of the panel
            id: statesLabel1
            anchors.horizontalCenter: parent.horizontalCenter
            states: [
                State {
                    name: "WRITE"
                    when: percussionPanelModel.currentPanelMode === PanelMode.WRITE
                    PropertyChanges {
                        target: statesLabel1
                        text: "WRITE MODE"
                    }
                },
                State {
                    name: "SOUND_PREVIEW"
                    when: percussionPanelModel.currentPanelMode === PanelMode.SOUND_PREVIEW
                    PropertyChanges {
                        target: statesLabel1
                        text: "SOUND PREVIEW MODE"
                    }
                },
                State {
                    name: "EDIT_LAYOUT"
                    when: percussionPanelModel.currentPanelMode === PanelMode.EDIT_LAYOUT
                    PropertyChanges {
                        target: statesLabel1
                        text: "EDIT LAYOUT MODE"
                    }
                }
            ]
        }
        StyledTextLabel {
            // Placeholder demonstrating instrument names / notation preview states
            id: statesLabel2
            anchors.horizontalCenter: parent.horizontalCenter
            states: [
                State {
                    name: "INSTRUMENT_NAMES"
                    when: !percussionPanelModel.useNotationPreview
                    PropertyChanges {
                        target: statesLabel2
                        text: "DISPLAYING INSTRUMENT NAMES"
                    }
                },
                State {
                    name: "NOTATION_PREVIEW"
                    when: percussionPanelModel.useNotationPreview
                    PropertyChanges {
                        target: statesLabel2
                        text: "DISPLAYING NOTATION PREVIEW"
                    }
                }
            ]
        }
    }
}
