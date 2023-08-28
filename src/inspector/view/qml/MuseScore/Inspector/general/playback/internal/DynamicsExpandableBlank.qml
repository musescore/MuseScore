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
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    property int navigationRowEnd: contentItem.navigationRowEnd

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: InspectorPropertyView {
        id: voicePropertyView

        readonly property int navigationRowEnd: applyToAllVoicesBtn.navigation.row + 1

        height: implicitHeight
        width: parent.width

        titleText: qsTrc("inspector", "Apply to voice")

        propertyItem: root.model ? root.model.applyToVoice : null

        navigationName: "Apply to voice"
        navigationPanel: root.navigation.panel
        navigationRowStart: root.navigation.row + 1

        RowLayout {
            height: childrenRect.height
            width: parent.width

            spacing: 4

            FlatRadioButtonList {
                id: voiceList

                Layout.fillWidth: true

                navigationPanel: voicePropertyView.navigationPanel
                navigationRowStart: voicePropertyView.navigationRowStart + 1

                spacing: 4

                currentValue: root.model.applyToVoice.value

                model: [
                    { iconCode: IconCode.VOICE_1, value: Dynamic.Voice1 },
                    { iconCode: IconCode.VOICE_2, value: Dynamic.Voice2 },
                    { iconCode: IconCode.VOICE_3, value: Dynamic.Voice3 },
                    { iconCode: IconCode.VOICE_4, value: Dynamic.Voice4 },
                ]

                onToggled: function(newValue) {
                    root.model.applyToVoice.value = newValue
                }
            }

            FlatRadioButton {
                id: applyToAllVoicesBtn

                Layout.preferredWidth: parent.width / 3

                navigation.name: "Apply to all voices"
                navigation.panel: voicePropertyView.navigationPanel
                navigation.row: voiceList.navigationRowEnd + 1

                text: qsTrc("inspector", "All")

                checked: root.model.applyToVoice.value === Dynamic.AllVoices

                onToggled: function(newValue) {
                    root.model.applyToVoice.value = Dynamic.AllVoices
                }
            }
        }
    }
}
