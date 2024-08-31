/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "LineSettings"

    width: parent.width
    spacing: 12

    function focusOnFirst() {
        tabBar.focusOnCurrentTab()
    }

    InspectorTabBar {
        id: tabBar

        InspectorTabButton {
            text: qsTrc("inspector", "Style")

            navigation.name: "StyleTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
        }

        InspectorTabButton {
            text: qsTrc("inspector", "Text")

            navigation.name: "TextTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }
    }

    StackLayout {
        width: parent.width
        currentIndex: tabBar.currentIndex

        height: itemAt(currentIndex).implicitHeight

        Loader {
            height: implicitHeight

            sourceComponent: {
                let  modelType = root.model ? root.model.modelType : -1

                switch (modelType) {
                case Inspector.TYPE_VOLTA: return voltaStyleSettings
                case Inspector.TYPE_OTTAVA: return ottavaStyleSettings
                case Inspector.TYPE_PEDAL: return pedalStyleSettings
                }

                return commonStyleSettings
            }
        }

        LineTextSettingsTab {
            height: implicitHeight

            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 2000
        }
    }

    Component {
        id: voltaStyleSettings

        VoltaStyleSettings {
            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }

    Component {
        id: ottavaStyleSettings

        OttavaStyleSettings {
            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }

    Component {
        id: pedalStyleSettings

        PedalStyleSettings {
            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }

    Component {
        id: commonStyleSettings

        LineWithHooksCommonStyleSettings {
            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }
}
