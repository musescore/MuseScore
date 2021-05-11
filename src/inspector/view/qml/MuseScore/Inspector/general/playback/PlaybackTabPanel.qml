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
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import "../../common"

TabPanel {
    id: root

    property QtObject proxyModel: null

    property NavigationPanel navigationPanel: null

    implicitHeight: Math.max(generalTab.visible ? generalTab.implicitHeight : 0,
                             dynamicsTab.visible ? dynamicsTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent.width

    TabItem {
        id: generalTab

        height: implicitHeight
        width: root.width

        navigation.name: "GeneralSettingsTab"
        navigation.panel: root.navigationPanel
        navigation.column: 1
        navigation.row: 1
        onNavigationTriggered: root.currentIndex = 0

        title: qsTrc("inspector", "General")

        GeneralSettingsTab {
            id: generalSettings

            navigationPanel: root.navigationPanel
            navigationColumn: 0
            navigationRowOffset: 2

            proxyModel: root.proxyModel

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width
        }
    }

    TabItem {
        id: dynamicsTab

        height: implicitHeight
        width: root.width

        navigation.name: "DynamicsSettingsTab"
        navigation.panel: root.navigationPanel
        navigation.column: 2
        navigation.row: 1
        onNavigationTriggered: root.currentIndex = 1

        title: qsTrc("inspector", "Dynamics")

        DynamicsSettingsTab {
            id: dynamicsSettings

            navigationPanel: root.navigationPanel
            navigationColumn: 2
            navigationRowOffset: 2

            proxyModel: root.proxyModel

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width
        }
    }
}
