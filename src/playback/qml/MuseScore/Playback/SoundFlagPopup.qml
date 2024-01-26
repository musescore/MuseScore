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

import "internal/SoundFlag"

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: navPanel.order

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    function updatePosition(elementRect) {
        var h = Math.max(root.contentHeight, 360)
        root.x = elementRect.x + elementRect.width + 12
        root.y = elementRect.y - h / 2
    }

    Column {
        id: content

        width: 300

        spacing: 12

        SoundFlagSettingsModel {
            id: soundFlagModel

            onItemRectChanged: function(rect) {
                updatePosition(rect)
            }
        }

        Component.onCompleted: {
            soundFlagModel.init()
        }

        NavigationPanel {
            id: navPanel
            name: "SoundFlagSettings"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Sound flag settings")
        }

        RowLayout {
            width: parent.width

            spacing: 6

            StyledIconLabel {
                iconCode: IconCode.AUDIO
            }

            StyledTextLabel {
                id: titleLabel

                Layout.fillWidth: true

                text: soundFlagModel.title
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }
        }

        Loader {
            id: loader

            width: parent.width
            height: Boolean(loader.item) ? loader.item.height : 0

            sourceComponent: {
                switch (soundFlagModel.sourceType) {
                case SoundFlagSettingsModel.MuseSounds:
                    return museSoundsComp
                case SoundFlagSettingsModel.VST:
                    return vstComp
                case SoundFlagSettingsModel.SoundFonts:
                    return soundFontsComp
                case SoundFlagSettingsModel.Undefined:
                    return undefined
                }
            }

            Component {
                id: museSoundsComp
                MuseSoundsParams {
                }
            }

            Component {
                id: vstComp
                VSTParams {
                }
            }

            Component {
                id: soundFontsComp
                SoundFontsParams {
                }
            }

        }

        SeparatorLine {}

        Column {
            width: parent.width

            spacing: 12

            CheckBox {
                id: showTextCheckbox
                text: qsTrc("notation", "Show text")
                checked: soundFlagModel.showText

                onClicked: {
                    soundFlagModel.showText = !checked
                }
            }

            TextInputField {
                Layout.preferredWidth: parent.width

                currentText: soundFlagModel.text
                visible: showTextCheckbox.checked

                onTextEditingFinished: function(newTextValue) {
                    soundFlagModel.text = newTextValue
                }
            }
        }
    }
}
