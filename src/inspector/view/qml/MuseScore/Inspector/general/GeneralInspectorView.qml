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
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"
import "playback"
import "appearance"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.height

    ColumnLayout {
        id: contentColumn

        width: parent.width

        spacing: 12

        GridLayout {
            id: grid

            width: parent.width

            columns: 2

            rowSpacing: 12
            columnSpacing: 4

            VisibilityBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.panel: root.navigationPanel
                navigation.name: "Visible"
                navigation.row: root.navigationRow(1)

                text: qsTrc("inspector", "Visible")

                isVisible: model && !model.isVisible.isUndefined ? model.isVisible.value : false

                onVisibleToggled: { model.isVisible.value = !model.isVisible.value }
            }

            VisibilityBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.panel: root.navigationPanel
                navigation.name: "Cue size"
                navigation.row: root.navigationRow(2)

                text: qsTrc("inspector", "Cue size")

                enabled: model ? model.isSmall.isEnabled : false
                isVisible: model && !model.isSmall.isUndefined ? model.isSmall.value : false

                onVisibleToggled: { model.isSmall.value = !model.isSmall.value }
            }

            VisibilityBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.panel: root.navigationPanel
                navigation.name: "Auto-place"
                navigation.row: root.navigationRow(3)

                text: qsTrc("inspector", "Auto-place")
                isVisible: model && !model.isAutoPlaceAllowed.isUndefined ? model.isAutoPlaceAllowed.value : false
                onVisibleToggled: { model.isAutoPlaceAllowed.value = !model.isAutoPlaceAllowed.value }
            }

            VisibilityBox {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width / 2

                navigation.panel: root.navigationPanel
                navigation.name: "Play"
                navigation.row: root.navigationRow(4)

                text: qsTrc("inspector", "Play")

                enabled: model ? model.isPlayable.isEnabled : false
                isVisible: model && !model.isPlayable.isUndefined ? model.isPlayable.value : false
                onVisibleToggled: { model.isPlayable.value = !model.isPlayable.value }
            }
        }

        Row {
            id: popupButtonsRow

            width: parent.width

            spacing: 4

            FlatButton {
                id: playbackButton

                width: (parent.width - popupButtonsRow.spacing)/ 2

                navigation.panel: root.navigationPanel
                navigation.name: "Playback"
                navigation.row: root.navigationRow(5)

                icon: IconCode.AUDIO
                text: qsTrc("inspector", "Playback")

                onClicked: {
                    if (playbackPopup.isOpened) {
                        playbackPopup.close()
                    } else {
                        playbackPopup.open()
                    }
                }

                PlaybackPopup {
                    id: playbackPopup
                    navigationParentControl: playbackButton.navigation
                    proxyModel: model ? model.playbackProxyModel : null
                }
            }

            FlatButton {
                id: appearanceButton

                width: (parent.width - popupButtonsRow.spacing)/ 2

                navigation.panel: root.navigationPanel
                navigation.name: "Appearance"
                navigation.row: root.navigationRow(6)

                icon: IconCode.POSITION_ARROWS
                text: qsTrc("inspector", "Appearance")

                onClicked: {
                    if (appearancePopup.isOpened) {
                        appearancePopup.close()
                    } else {
                        appearancePopup.open()
                    }
                }

                AppearancePopup {
                    id: appearancePopup
                    navigationParentControl: appearanceButton.navigation
                    model: root.model ? root.model.appearanceSettingsModel : null
                }
            }
        }
    }
}
