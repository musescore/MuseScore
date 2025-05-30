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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.Inspector 1.0

import "../../common"

StyledPopupView {
    id: root

    required property QtObject textSettingsModel

    property int navigationOrderStart: 0
    readonly property int navigationOrderEnd: frameSettingsNavPanel.order

    height: contentHeight

    NavigationPanel {
        id: frameSettingsNavPanel
        name: "FrameSettings"
        direction: NavigationPanel.Vertical
        section: root.navigationSection
        order: root.navigationOrderStart
        accessible.name: qsTrc("inspector", "Frame settings")
    }

    FrameSettings {
        id: frameSettings

        height: implicitHeight

        navigationPanel: frameSettingsNavPanel
        navigationRowStart: 1

        frameType: root.textSettingsModel.frameType
        frameBorderColor: root.textSettingsModel.frameBorderColor
        frameFillColor: root.textSettingsModel.frameFillColor
        frameThickness: root.textSettingsModel.frameThickness
        frameMargin: root.textSettingsModel.frameMargin
        frameCornerRadius: root.textSettingsModel.frameCornerRadius
    }
}
