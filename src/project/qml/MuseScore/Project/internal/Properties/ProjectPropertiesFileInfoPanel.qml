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

ColumnLayout {
    id: root

    Layout.fillWidth: true

    spacing: 4

    property int propertyNameWidth: 110

    property NavigationPanel navigationPanel: null
    property int navigationColumnEnd: apiLevelProperty.navigationColumnEnd

    RowLayout {
        spacing: 8

        PropertyItem {
            id: filePathProperty

            propertyNameWidth: root.propertyNameWidth
            index: 0
            propertyName: qsTrc("project/properties", "File path:")
            propertyValue: projectPropertiesModel.filePath
            isFileInfoPanelProperty: true
            valueFillWidth: true

            navigationPanel: root.navigationPanel
        }

        FlatButton {
            id: openFileLocation

            Layout.preferredWidth: 30
            Layout.preferredHeight: 30

            icon: IconCode.OPEN_FILE

            navigation.name: "OpenFileLocation"
            navigation.panel: root.navigationPanel
            navigation.column: filePathProperty.navigationColumnEnd + 1
            accessible.name: "Open file location"

            onClicked: projectPropertiesModel.openFileLocation()
        }

        Item { width: 8 }
    }

    RowLayout {
        Layout.topMargin: 4

        spacing: 8

        PropertyItem {
            index: 2
            propertyName: qsTrc("project/properties", "MuseScore version:")
            propertyValue: projectPropertiesModel.version
            isFileInfoPanelProperty: true

            navigationPanel: root.navigationPanel
        }

        PropertyItem {
            index: 3
            propertyName: qsTrc("project/properties", "Revision:")
            propertyValue: projectPropertiesModel.revision
            isFileInfoPanelProperty: true

            Layout.leftMargin: 32
            Layout.rightMargin: 32

            navigationPanel: root.navigationPanel
        }

        PropertyItem {
            id: apiLevelProperty

            index: 4
            propertyName: qsTrc("project/properties", "API-Level:")
            propertyValue: projectPropertiesModel.apiLevel
            isFileInfoPanelProperty: true

            navigationPanel: root.navigationPanel
        }
    }
}
