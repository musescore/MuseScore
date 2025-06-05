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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    property alias navigation: navPanel

    anchors.fill: parent

    NavigationPanel {
        id: navPanel
        name: "MixerPanelToolbarPanel"
        enabled: root.enabled && root.visible
    }

    // TODO: https://github.com/musescore/MuseScore/issues/16722
    /*
    FlatButton {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 2

        icon: IconCode.SETTINGS_COG
        text: qsTrc("playback", "Customize mixer")
        orientation: Qt.Horizontal

        navigation.panel: navPanel
        navigation.row: 0
    }
    */
}
