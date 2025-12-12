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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../common"
import "internal"

Column {
    id: root

    required property LyricsLineSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "LyricsLineSettings"

    spacing: 12

    function focusOnFirst() {
        thicknessSection.focusOnFirst()
    }

    SpinBoxPropertyView {
        id: thicknessSection

        titleText: qsTrc("inspector", "Thickness")
        propertyItem: model ? model.thickness : null

        step: 0.01
        maxValue: 10.00
        minValue: 0.01
        decimals: 2

        navigationName: "Thickness"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    SpinBoxPropertyView {
        id: setVerse
        titleText: qsTrc("inspector", "Set to verse")
        propertyItem: root.model ? root.model.verse : null

        visible: model ? model.hasVerse : false

        decimals: 0
        step: 1
        minValue: 1

        navigationName: "Verse"
        navigationPanel: root.navigationPanel
        navigationRowStart: thicknessSection.navigationRowEnd + 1
    }

}
