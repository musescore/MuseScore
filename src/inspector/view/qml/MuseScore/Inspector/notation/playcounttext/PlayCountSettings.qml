/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    objectName: "PlayCountSettings"

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    property QtObject model: null

    spacing: 12

    width: parent.width

    FlatRadioButtonGroupPropertyView {
        id: playCountTextSection
        propertyItem: root.model ? root.model.playCountTextSetting : null

        showTitle: true;
        titleLabelComponent: Component {
            id: playCountTextSectionLabel

            StyledTextLabel {
                width: parent.width
                text: qsTrc("inspector", "Play count text")
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideNone
                wrapMode: Text.Wrap
            }
        }

        navigationName: "Play count text"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "Auto"), value: BarlineTypes.COUNT_AUTO },
            { text: qsTrc("inspector", "Custom"), value: BarlineTypes.COUNT_CUSTOM },
            { text: qsTrc("inspector", "Hide"), value: BarlineTypes.COUNT_HIDE }
        ]
    }

    TextSection {
        id: playCountText
        propertyItem: root.model ? root.model.playCountText : null
        visible: root.model && root.model.playCountTextSetting.value && root.model.playCountTextSetting.value === BarlineTypes.COUNT_CUSTOM

        showButton: false

        navigationPanel: root.navigationPanel
        navigationRowStart: playCountTextSection.navigationRowStart + 1
    }
}
