/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

        frameType: root.textSettingsModel ? root.textSettingsModel.frameType : null
        frameBorderColor: root.textSettingsModel ? root.textSettingsModel.frameBorderColor : null
        frameFillColor: root.textSettingsModel ? root.textSettingsModel.frameFillColor : null
        frameThickness: root.textSettingsModel ? root.textSettingsModel.frameThickness : null
        frameMargin: root.textSettingsModel ? root.textSettingsModel.frameMargin : null
        frameCornerRadius: root.textSettingsModel ? root.textSettingsModel.frameCornerRadius : null
    }
}
