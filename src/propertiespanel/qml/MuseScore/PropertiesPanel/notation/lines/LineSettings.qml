/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
pragma ComponentBehavior: Bound 

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

import "../../common"
import "internal"

Column {
    id: root

    required property TextLineSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "LineSettings"

    width: parent.width
    spacing: 12

    function focusOnFirst() {
        tabBar.focusOnCurrentTab()
    }

    PropertiesPanelTabBar {
        id: tabBar

        PropertiesPanelTabButton {
            text: qsTrc("propertiespanel", "Style")
            maxWidth: tabBar.truncatedItemWidth

            navigation.name: "StyleTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
        }

        PropertiesPanelTabButton {
            text: qsTrc("propertiespanel", "Text")
            maxWidth: tabBar.truncatedItemWidth

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
                case PropertiesPanelAbstractModel.TYPE_VOLTA: return voltaStyleSettings
                case PropertiesPanelAbstractModel.TYPE_OTTAVA: return ottavaStyleSettings
                case PropertiesPanelAbstractModel.TYPE_PEDAL: return pedalStyleSettings
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
            model: root.model as VoltaSettingsModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }

    Component {
        id: ottavaStyleSettings

        OttavaStyleSettings {
            model: root.model as OttavaSettingsModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }
    }

    Component {
        id: pedalStyleSettings

        PedalStyleSettings {
            model: root.model as PedalSettingsModel

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
