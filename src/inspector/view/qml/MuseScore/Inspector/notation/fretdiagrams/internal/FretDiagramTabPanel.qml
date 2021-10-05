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
import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import "../../../common"

TabPanel {
    id: root

    property QtObject model: null

    property int navigationRowStart: 1

    implicitHeight: Math.max(generalTab.visible ? generalTab.implicitHeight : 0,
                             advancedTab.visible ? advancedTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent ? parent.width : 0

    function focusOnFirst() {
        generalTab.navigation.requestActive()
    }

    TabItem {
        id: generalTab

        title: qsTrc("inspector", "General")
        checked: root.currentIndex === 0

        navigation.name: "GeneralTab"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart
        onNavigationTriggered: root.currentIndex = 0

        FretGeneralSettingsTab {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            enabled: generalTab.checked

            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }

    TabItem {
        id: advancedTab

        title: qsTrc("inspector", "Settings")
        checked: root.currentIndex === 1

        enabled: root.model ? root.model.areSettingsAvailable : false

        navigation.name: "SettingsTab"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
        onNavigationTriggered: root.currentIndex = 1

        FretAdvancedSettingsTab {
            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            enabled: advancedTab.checked

            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 2000
        }
    }
}
