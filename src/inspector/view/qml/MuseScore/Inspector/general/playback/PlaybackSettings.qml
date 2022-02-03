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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "../../common"

Column {
    id: root

    property QtObject proxyModel: null

    property NavigationPanel navigationPanel: null

    width: parent.width
    spacing: 12

    Connections {
        target: proxyModel

        function onIsEmptyChanged() {
            var hasGeneralSettings = proxyModel.hasGeneralSettings
            var hasDynamicsSettings = proxyModel.hasDynamicsSettings

            if (hasGeneralSettings && !hasDynamicsSettings) {
                tabBar.currentIndex = 0
            } else if (!hasGeneralSettings && hasDynamicsSettings) {
                tabBar.currentIndex = 1
            }
        }
    }

    function forceFocusIn() {
        tabBar.focusOnCurrentTab()
    }

    InspectorTabBar {
        id: tabBar

        InspectorTabButton {
            text: qsTrc("inspector", "General")

            navigation.name: "GeneralSettingsTab"
            navigation.panel: root.navigationPanel
            navigation.row: 1
        }

        InspectorTabButton {
            text: qsTrc("inspector", "Dynamics")

            navigation.name: "DynamicsSettingsTab"
            navigation.panel: root.navigationPanel
            navigation.row: 2
        }
    }

    StackLayout {
        width: parent.width
        currentIndex: tabBar.currentIndex

        height: itemAt(currentIndex).implicitHeight

        PlaybackGeneralSettings {
            height: implicitHeight

            proxyModel: root.proxyModel

            navigationPanel: root.navigationPanel
            navigationRowStart: 1000
        }

        PlaybackDynamicsSettings {
            height: implicitHeight

            proxyModel: root.proxyModel

            navigationPanel: root.navigationPanel
            navigationRowStart: 2000
        }
    }
}
